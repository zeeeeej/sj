#ifndef IMAGE_INFO_LIST_H
#define IMAGE_INFO_LIST_H

#include <pthread.h>
#include <limits.h>

#define PATH_MAX 1024

// 定义结构体
typedef struct {
    unsigned char id;
    unsigned char trigger_type;
    unsigned char trigger_angle;
    unsigned int capture_time;
    unsigned int image_length;
    unsigned char md5[33];
    char file_path[PATH_MAX]; // 存储文件路径
} DataNode;

typedef struct __attribute__((packed)) {
    unsigned char id;
    unsigned char trigger_type;
    unsigned char trigger_angle;
    unsigned int capture_time;
    unsigned int image_length;
    unsigned char md5[16]; // 注意这里只保留16字节的md5值
} BufferedDataNode;

// 定义链表节点
typedef struct Node {
    DataNode data;
    struct Node* next;
} ListNode;

// 链表头结点和互斥锁
typedef struct ThreadSafeList {
    ListNode* head;
    pthread_mutex_t lock;
} ThreadSafeList;




// 初始化链表
struct ThreadSafeList* initList();

// 创建新节点
void appendNode(struct ThreadSafeList* list, DataNode data);

// 打印链表中的所有节点
void printList(struct ThreadSafeList* list);

// 释放链表内存
void freeList(struct ThreadSafeList* list);

// 向陀螺仪触发链表添加节点
void appendToGyroscopeTriggerList(DataNode data);

// 向主动触发链表添加节点
void appendToActiveTriggerList(DataNode data);

// //根据ID删除节点
// void deleteNodeById(struct ThreadSafeList* list, unsigned char id);


int deleteListNodeById(unsigned char id);


// 根据id获取链表中的节点
DataNode* getNodeById(struct ThreadSafeList* list, unsigned char id);

// 根据文件路径生成图片信息
void generate_image_info(char *image_path);

BufferedDataNode* copy_both_lists_to_buffer(size_t *buffer_size);

int find_file_path_by_id(unsigned char id, char *file_path);

void cleanAllLinkList();
void deleteNodeByFileName(const char *file_path);

void printAllListlist();
#endif // THREAD_SAFE_LIST_H