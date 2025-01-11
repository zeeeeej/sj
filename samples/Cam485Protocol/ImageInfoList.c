#include "ImageInfoList.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <openssl/md5.h>
#include <unistd.h>
#include <libgen.h> // for basename

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
ThreadSafeList* initList() {
    ThreadSafeList* list = (ThreadSafeList*)malloc(sizeof(ThreadSafeList));
    if (!list) {
        printf("内存分配失败\n");
        exit(1);
    }
    list->head = NULL;
    pthread_mutex_init(&list->lock, NULL);
    return list;
}

// 创建新节点
ListNode* createNode(DataNode data) {
    ListNode* newNode = (ListNode*)malloc(sizeof(ListNode));
    if (!newNode) {
        printf("内存分配失败\n");
        exit(1);
    }
    newNode->data = data;
    newNode->next = NULL;
    return newNode;
}

// 在链表尾部添加节点
void appendNode(ThreadSafeList* list, DataNode data) {
    pthread_mutex_lock(&list->lock);
    ListNode* newNode = createNode(data);
    if (list->head == NULL) {
        list->head = newNode;
    } else {
        ListNode* temp = list->head;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = newNode;
    }
    pthread_mutex_unlock(&list->lock);
}

// 打印链表中的所有节点
void printList(ThreadSafeList* list) {
    pthread_mutex_lock(&list->lock);
    ListNode* temp = list->head;
    while (temp != NULL) {
        printf("ID: %d, Trigger Type: %d, Trigger Angle: %d, Capture Time: %u, Image Length: %u, MD5: ",
               temp->data.id, temp->data.trigger_type, temp->data.trigger_angle,
               temp->data.capture_time, temp->data.image_length);
        for (int i = 0; i < 16; i++) {
            printf("%02X", temp->data.md5[i]);
        }
        printf(", File Path: %s\n", temp->data.file_path);
        temp = temp->next;
    }
    pthread_mutex_unlock(&list->lock);
}

// 释放链表内存
void freeList(ThreadSafeList* list) {
    pthread_mutex_destroy(&list->lock);
    ListNode* current = list->head;
    ListNode* next;
    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
    free(list);
}

// 静态变量表示两个链表
static ThreadSafeList* gyroscopeTriggerList;
static ThreadSafeList* activeTriggerList;

// 向陀螺仪触发链表添加节点
void appendToGyroscopeTriggerList(DataNode data) {
    if (gyroscopeTriggerList == NULL) {
        gyroscopeTriggerList = initList();
    }
    appendNode(gyroscopeTriggerList, data);
}

// 向主动触发链表添加节点
void appendToActiveTriggerList(DataNode data) {
    if (activeTriggerList == NULL) {
        activeTriggerList = initList();
    }
    appendNode(activeTriggerList, data);
}

// // 计算文件的MD5哈希值
// void calculate_md5(const char *filename, unsigned char *md5_result) {
//     FILE *file = fopen(filename, "rb");
//     if (!file) {
//         perror("无法打开文件");
//         return;
//     }

//     MD5_CTX c;
//     int bytes;
//     unsigned char data[1024];

//     MD5_Init(&c);
//     while ((bytes = fread(data, 1, sizeof(data), file)) != 0)
//         MD5_Update(&c, data, bytes);
//     MD5_Final(md5_result, &c);

//     fclose(file);
// }

// 解析文件名以获取触发类型和触发角度
void parse_filename(const char *filename, unsigned char *trigger_type, unsigned char *trigger_angle) {
    // 这里假设文件名格式为 "type_angle.jpg" 或类似的格式
    sscanf(filename, "%hhu_%hhu", trigger_type, trigger_angle);
}

// 根据文件路径生成图片信息
void generate_image_info(char *image_path) {
    struct stat file_stat;
    if (stat(image_path, &file_stat) != 0) {
        perror("无法获取文件状态");
        return;
    }

    // 获取文件大小
    unsigned int image_length = file_stat.st_size;

    // 获取抓取时间（文件修改时间）
    unsigned int capture_time = file_stat.st_mtime;

    // 计算MD5哈希值
    unsigned char md5[16];
    calculate_file_md5(image_path, md5);

    // 解析文件名以获取触发类型和触发角度
    unsigned char trigger_type = 0;
    unsigned char trigger_angle = 0;
    parse_filename(basename(strdup(image_path)), &trigger_type, &trigger_angle);

    // 假设pic_id是一个自增的唯一标识符
    static unsigned char pic_id_counter = 0;
    unsigned char pic_id = ++pic_id_counter;

    // 构建DataNode结构体
    DataNode data = {
        .id = pic_id,
        .trigger_type = trigger_type,
        .trigger_angle = trigger_angle,
        .capture_time = capture_time,
        .image_length = image_length,
    };
    memcpy(data.md5, md5, sizeof(md5));
    strncpy(data.file_path, image_path, PATH_MAX - 1);
    data.file_path[PATH_MAX - 1] = '\0'; // 确保字符串结束

    // 根据触发类型决定添加到哪个链表
    if (trigger_type == 1) {
        appendToGyroscopeTriggerList(data);
    } else if (trigger_type == 2) {
        appendToActiveTriggerList(data);
    } else {
        printf("未知的触发类型: %d\n", trigger_type);
    }
}