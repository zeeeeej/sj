#ifndef __NODE_LIST_H__
#define __NODE_LIST_H__

#include "list.h"

#ifdef __cplusplus
extern "C"{
#endif

typedef struct node {
	struct list_head head;
	void *data;			/* need alloc memory */
	unsigned int size;
	nl_context_t context;		/* context of node belongs */
	int priv;			/* private data */
} node_t;

void Node_List_Set_Block(nl_context_t context, int block);

/**
 * used for node producer
 **/
node_t *Get_Free_Node(nl_context_t context);
void Put_Use_Node(nl_context_t context, node_t *node);

/**
 * used for node consumer
 **/
node_t *Get_Use_Node(nl_context_t context);
void Put_Free_Node(nl_context_t context, node_t *node);
void Drop_Use_Node(nl_context_t context);

/**
 * node ops
 **/
void Set_Node_Private(node_t *node, int _private);

/**
 * Init
 * node_size: node buf size;
 * nodes: number of nodes, must >= 2;
 * keep_update: keep update the use list, 1: keep update use list, 0: keep old data;
 * block: if use list empty, get use node return NULL.
 * Return: context, 0 is fail
 **/
nl_context_t Node_List_Init(unsigned int node_size,
			    unsigned int nodes,
			    int keep_update, int block);

/**
 * Deinit
 * Return: 0 if success
 **/
int Node_List_Deinit(nl_context_t context);

#ifdef __cplusplus
}
#endif

#endif /* __NODE_LIST_H__ */
