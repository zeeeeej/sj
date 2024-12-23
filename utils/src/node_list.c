/* node_list.c
 *
 * node list manager for spi slv and evt driver;
 *
 * Copyright (c) 2016 Ingenic
 * Author:Elvis <huan.wang@ingenic.com>
 *
 * Version v1.0: node list init version
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>

#include <node_list.h>

#ifdef __cplusplus
extern "C"{
#endif

typedef struct node_list {
	/* free list */
	unsigned int free_cnt;	/* for futher use */
	struct list_head free;
	sem_t free_sem;
	pthread_mutex_t free_mutex;
	/* use list */
	unsigned int use_cnt;	/* for futher use */
	struct list_head use;
	sem_t use_sem;
	pthread_mutex_t use_mutex;
	/* other */
	int keep_update;
	volatile int block;
	void *node_mem;
	void *data_mem;
	unsigned int node_size;
} node_list_t;

node_t *Get_Use_Node(nl_context_t context);

/**********************************
 *  rx_node_list
 *********************************/
node_t *Get_Free_Node(nl_context_t context)
{
	if (context) {
		node_list_t *list = (node_list_t *)context;
		node_t *node = NULL;
		struct list_head *head = &list->free;

		if (list_empty(head) && list->keep_update) {
			node = Get_Use_Node(context);
			if (!node) {
				printf("FATAL ERROR: lost nodes\n");
				return NULL;
			}
			memset(node->data, 0, list->node_size);
			return node;
		} else
			sem_wait(&list->free_sem);

		pthread_mutex_lock(&list->free_mutex);
		{
			node = list_entry(head->_next, node_t, head);
			list_del(head->_next);
			list->free_cnt--;
		}
		pthread_mutex_unlock(&list->free_mutex);

		memset(node->data, 0, list->node_size);
		return node;
	}

	return NULL;
}

void Put_Use_Node(nl_context_t context, node_t *node)
{
	if (context) {
		node_list_t *list = (node_list_t *)context;
		struct list_head *head = &list->use;

		if (node == NULL) {
			printf("%s: ERROR node can't be NULL\n", __func__);
			return;
		}

		pthread_mutex_lock(&list->use_mutex);
		{
			list_add_tail(&node->head, head);
			list->use_cnt++;
			sem_post(&list->use_sem);
		}
		pthread_mutex_unlock(&list->use_mutex);
	}
}

/**
 * blocked get rx use node
 **/
node_t *Get_Use_Node(nl_context_t context)
{
	if (context) {
		node_list_t *list = (node_list_t *)context;
		node_t *node = NULL;
		struct list_head *head = &list->use;

		if (list_empty(head) && !list->block)
			return NULL;
		else {
			sem_wait(&list->use_sem);
			if (list_empty(head)) {
				return NULL;
			}
		}

		pthread_mutex_lock(&list->use_mutex);
		{
			node = list_entry(head->_next, node_t, head);
			list_del(head->_next);
			list->use_cnt--;
		}
		pthread_mutex_unlock(&list->use_mutex);

		return node;
	}

	return NULL;
}

void Put_Free_Node(nl_context_t context, node_t *node)
{
	if (context) {
		node_list_t *list = (node_list_t *)context;
		struct list_head *head = &list->free;

		if (node == NULL) {
			printf("%s: ERROR node can't be NULL\n", __func__);
			return;
		}
		pthread_mutex_lock(&list->free_mutex);
		{
			list_add_tail(&node->head, head);
			list->free_cnt++;
			sem_post(&list->free_sem);
		}
		pthread_mutex_unlock(&list->free_mutex);
	}
}

void Drop_Use_Node(nl_context_t context)
{
	if (context) {
		node_list_t *list = (node_list_t *)context;
		struct list_head *head = &list->use;
		node_t *node = NULL;

		while (!list_empty(head)) {
			pthread_mutex_lock(&list->use_mutex);
			{
				node = list_entry(head->_next, node_t, head);
				list_del(head->_next);
				list->use_cnt--;
			}
			pthread_mutex_unlock(&list->use_mutex);

			Put_Free_Node(context, node);
		}
	}
}

/**
 * node ops
 **/
void Set_Node_Private(node_t *node, int private)
{
	node->priv = private;
}

/**
 * node_size: node buf size
 * nodes: number of nodes
 * keep_update: keep update the use list
 **/
nl_context_t Node_List_Init(unsigned int node_size,
			    unsigned int nodes,
			    int keep_update, int block)
{
	int i;
	node_list_t *list = NULL;
	void *node_mem = NULL;
	void *data_mem = NULL;

	/* check params */
	if ((node_size <= 0) || (nodes < 2)) {
		printf("%s: ERROR: argument error!\n", __func__);
		return 0;
	}

	/* alloc context */
	list = malloc(sizeof(node_list_t));
	if (!list) {
		printf("%s: ERROR: alloc context error!\n", __func__);
		return 0;
	}
	memset(list, 0x0, sizeof(node_list_t));

	/* init list head */
	INIT_LIST_HEAD(&list->free);
	INIT_LIST_HEAD(&list->use);

	/* init sem and mutex */
	sem_init(&list->free_sem, 0, 0);
	sem_init(&list->use_sem, 0, 0);
	pthread_mutex_init(&list->free_mutex, NULL);
	pthread_mutex_init(&list->use_mutex, NULL);

	/* alloc node and data */
	node_mem = malloc(nodes * sizeof(node_t));
	if (!node_mem) {
		printf("%s: ERROR: alloc memory for node error!\n", __func__);
		goto error1;
	}
	memset(node_mem, 0x0, nodes * sizeof(node_t));

	data_mem = malloc(nodes * node_size);
	if (!data_mem) {
		printf("%s: ERROR: alloc memory for data error!\n", __func__);
		goto error2;
	}
	memset(data_mem, 0x0, nodes * node_size);

	/* store node_mem and data_mem */
	list->node_mem = node_mem;
	list->data_mem = data_mem;
	list->node_size = node_size;
	list->keep_update = keep_update;
	list->block = block;

	for (i = 0; i < nodes; i++) {
		node_t *node = (node_t *)((char *)node_mem + i * sizeof(node_t));
		node->data = (char *)data_mem + i * node_size;
		node->context = (nl_context_t)list;
		node->priv = 0;
		Put_Free_Node((nl_context_t)list, node);
	}

	return (nl_context_t)list;

error2:
	free(node_mem);
error1:
	free(list);

	return 0;
}

void Node_List_Set_Block(nl_context_t context, int block)
{
	if (context) {
		node_list_t *list = (node_list_t *)context;
		if (list->block && !block) {
			int sem_value;
			list->block = 0;
			sem_getvalue(&list->use_sem, &sem_value);
			if (sem_value == 0) {
				sem_post(&list->use_sem);
			}
		} else if (!list->block && block) {
			list->block = 1;
		}
	}
}

int Node_List_Deinit(nl_context_t context)
{
	if (context) {
		node_list_t *list = (node_list_t *)context;
		free(list->data_mem);
		free(list->node_mem);
		free(list);
	}

	return 0;
}

#ifdef __cplusplus
}
#endif
