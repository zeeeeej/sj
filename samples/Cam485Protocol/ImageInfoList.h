#ifndef IMAGE_INFO_LIST_H
#define IMAGE_INFO_LIST_H

#include <pthread.h>
#include <limits.h>

// 定义结构体
typedef struct {
    unsigned char id;
    unsigned char trigger_type;
    unsigned char trigger_angle;
    unsigned int capture_time;
    unsigned int image_length;
    unsigned char md5[16];
    char file_path[PATH_MAX]; // 存储文件路径
} DataNode;

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

// 根据文件路径生成图片信息
void generate_image_info(char *image_path);

#endif // THREAD_SAFE_LIST_H