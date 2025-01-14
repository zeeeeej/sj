#include "ImageInfoList.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <libgen.h> // for basename
#include <limits.h>
#include "md5.h"
#include "ProtocolPort.h"
#include "cm_common.h"


ThreadSafeList* gyroscopeTriggerList = NULL;
ThreadSafeList* activeTriggerList    = NULL;


// 初始化链表
ThreadSafeList* initList() {
    ThreadSafeList* list = (ThreadSafeList*)malloc(sizeof(ThreadSafeList));
    if (!list) {
        printf("mem malloc error\n");
        exit(1);
    }
    list->head = NULL;
    pthread_mutex_init(&list->lock, NULL);
    return list;
}
/**
 * 遍历链表并将每个节点的信息拷贝到缓冲区中
 * @param list 链表指针
 * @param buffer_size 输出缓冲区大小（以元素为单位）
 * @return 分配并填充好的缓冲区，或 NULL 表示失败
 */
BufferedDataNode* copy_list_to_buffer(ThreadSafeList *list, size_t *buffer_size) {
    if (list == NULL || list->head == NULL) {
        printf("The list is empty or invalid.\n");
        *buffer_size = 0;
        return NULL;
    }

    // 加锁以确保线程安全
    pthread_mutex_lock(&list->lock);

    // 计算链表长度
    size_t count = 0;
    ListNode *current = list->head;
    while (current != NULL) {
        count++;
        current = current->next;
    }
    LOGD("count : %d\n",count);
    // 分配缓冲区
    BufferedDataNode *buffer = (BufferedDataNode *)malloc(count * sizeof(BufferedDataNode));
    if (buffer == NULL) {
        pthread_mutex_unlock(&list->lock);
        printf("Failed to allocate memory for buffer.\n");
        *buffer_size = 0;
        return NULL;
    }
    *buffer_size = count;

    // 再次遍历链表，拷贝数据到缓冲区
    current = list->head;
    for (size_t i = 0; i < count; ++i) {
        memcpy(buffer[i].md5, current->data.md5, 16); // 只拷贝16字节的md5值
        buffer[i].id = current->data.id;
        buffer[i].trigger_type = current->data.trigger_type;
        buffer[i].trigger_angle = current->data.trigger_angle;
        buffer[i].capture_time = current->data.capture_time;
        buffer[i].image_length = current->data.image_length;
        current = current->next;
    }

    // 解锁
    pthread_mutex_unlock(&list->lock);

    return buffer;
}
// 创建新节点
ListNode* createNode(DataNode data) {
    ListNode* newNode = (ListNode*)malloc(sizeof(ListNode));
    if (!newNode) {
        printf("mem malloc erro \n");
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


/**
 * 拷贝两个链表的所有节点信息到缓冲区中
 * @param buffer_size 输出缓冲区大小（以元素为单位）
 * @return 分配并填充好的缓冲区，或 NULL 表示失败
 */
BufferedDataNode* copy_both_lists_to_buffer(size_t *buffer_size) {
    // 初始化计数器和缓冲区
    size_t gyro_count = 0;
    size_t active_count = 0;
    BufferedDataNode *gyro_buffer = copy_list_to_buffer(gyroscopeTriggerList, &gyro_count);
    BufferedDataNode *active_buffer = copy_list_to_buffer(activeTriggerList, &active_count);

    if (gyro_buffer == NULL && active_buffer == NULL) {
        *buffer_size = 0;
        return NULL;
    }

    // 计算总大小并分配最终缓冲区
    *buffer_size = gyro_count + active_count;
    BufferedDataNode *combined_buffer = (BufferedDataNode *)malloc(*buffer_size * sizeof(BufferedDataNode));
    if (combined_buffer == NULL) {
        free(gyro_buffer);
        free(active_buffer);
        printf("Failed to allocate memory for combined buffer.\n");
        *buffer_size = 0;
        return NULL;
    }

    // 将数据拷贝到最终缓冲区
    if (gyro_buffer != NULL) {
        memcpy(combined_buffer, gyro_buffer, gyro_count * sizeof(BufferedDataNode));
        free(gyro_buffer);
    }
    if (active_buffer != NULL) {
        memcpy(combined_buffer + gyro_count, active_buffer, active_count * sizeof(BufferedDataNode));
        free(active_buffer);
    }

    return combined_buffer;
}


/**
 * 获取单个链表的节点数量
 * @param list 链表指针
 * @return 节点数量
 */
static size_t get_list_node_count(ThreadSafeList *list) {
    if (list == NULL || list->head == NULL) {
        return 0;
    }

    // 加锁以确保线程安全
    pthread_mutex_lock(&list->lock);

    // 计算链表长度
    size_t count = 0;
    ListNode *current = list->head;
    while (current != NULL) {
        count++;
        current = current->next;
    }

    // 解锁
    pthread_mutex_unlock(&list->lock);

    return count;
}

/**
 * 获取两个链表的所有节点总数
 * @return 节点总数
 */
uint8_t  get_total_node_count(void) {
    // 获取每个链表的节点数量
    uint8_t gyro_count = get_list_node_count(gyroscopeTriggerList);
    uint8_t active_count = get_list_node_count(activeTriggerList);

    // 返回两个链表的节点总数
    return gyro_count + active_count;
}
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

void deleteNodeById(ThreadSafeList *list, unsigned char id)
{
   if (list == NULL || list->head == NULL) {
        return; // 链表为空，直接返回
    }

    // 加锁以确保线程安全
    pthread_mutex_lock(&list->lock);

    ListNode *current = list->head;
    ListNode *prev = NULL;

    while (current != NULL) {
        // 判断当前节点的数据 id 是否匹配
        if (current->data.id == id) {
            // 如果匹配，释放当前节点
            if (prev == NULL) {
                // 删除的是头节点
                list->head = current->next;
            } else {
                // 删除的是中间或尾部节点
                prev->next = current->next;
            }

            // 如果 DataNode 有动态分配的内容（如文件路径），需要额外释放
            // 这里的 file_path 和 md5 是静态分配的，不需要额外释放
            free(current); // 释放当前节点
            pthread_mutex_unlock(&list->lock); // 解锁并退出
            return;
        }
        // 继续遍历
        prev = current;
        current = current->next;
    }

    // 未找到匹配的节点
    pthread_mutex_unlock(&list->lock);
}

void deleteAllNodes(ThreadSafeList *list)
{
    if (list == NULL || list->head == NULL) {
        return;
    }

    // 加锁以确保线程安全
    pthread_mutex_lock(&list->lock);

    ListNode *current = list->head;
    ListNode *next;
    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
    list->head = NULL;

    // 解锁
    pthread_mutex_unlock(&list->lock);
}

DataNode *getNodeById(ThreadSafeList *list, unsigned char id)
{
    pthread_mutex_lock(&list->lock);
    if (list == NULL || list->head == NULL) {
        return NULL;
    }
    DataNode *data = NULL;
    ListNode *current = list->head;
    ListNode *prev = NULL;
    while (current != NULL)
    {
        if (current->data.id == id)
        {
            data = &current->data;
            break;
        }
        prev = current;
        current = current->next;
    }
    return data;
}


/**
 * 在单个链表中查找指定 id 对应的文件路径
 * @param list 链表指针
 * @param id 要查找的节点 id
 * @param file_path 用于存储找到的文件路径
 * @return 成功返回0，未找到返回-1
 */
static int find_in_list(ThreadSafeList *list, unsigned char id, char *file_path) {
    if (list == NULL || list->head == NULL) {
        return -1;
    }

    // 加锁以确保线程安全
    pthread_mutex_lock(&list->lock);

    ListNode *current = list->head;
    while (current != NULL) {
        if (current->data.id == id) {
            strncpy(file_path, current->data.file_path, PATH_MAX);
            file_path[PATH_MAX - 1] = '\0'; // 确保字符串以null结尾
            pthread_mutex_unlock(&list->lock);
            return 0;
        }
        current = current->next;
    }

    // 解锁
    pthread_mutex_unlock(&list->lock);

    return -1; // 未找到
}
/**
 * 查找指定 id 对应的文件路径
 * @param id 要查找的节点 id
 * @param file_path 用于存储找到的文件路径
 * @return 成功返回0，未找到返回-1
 */
int find_file_path_by_id(unsigned char id, char *file_path) {
    if (file_path == NULL) {
        return -1;
    }
    *file_path = '\0'; // 初始化为空字符串

    // 辅助函数：在单个链表中查找
    int find_in_list(ThreadSafeList *list, unsigned char id, char *file_path);

    // 先在 gyroscopeTriggerList 中查找
    if (find_in_list(gyroscopeTriggerList, id, file_path) == 0) {
        return 0; // 找到并返回
    }

    // 如果没有找到，则在 activeTriggerList 中查找
    if (find_in_list(activeTriggerList, id, file_path) == 0) {
        return 0; // 找到并返回
    }

    // 如果两个链表都没有找到
    return -1;
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

/**
 * 解析文件名并提取触发类型、ID和角度信息
 * @param filepath 文件路径字符串
 * @param trigger_type 解析出的触发类型
 * @param id 解析出的ID值
 * @param angle 解析出的角度值（对于主动拍照目录下的文件，angle设置为-1）
 * @return 成功返回0，失败返回-1
 */
int parse_filename(const char *filepath, unsigned char *trigger_type, unsigned char *id, unsigned char *angle) {
    char full_path[PATH_MAX];
    strncpy(full_path, filepath, sizeof(full_path) - 1);
    full_path[sizeof(full_path) - 1] = '\0';

    // 获取目录路径和文件名
    const char *dir_path;
    char *filename; // 使用 char * 而不是 const char *，因为我们将在副本上进行修改

    filename = strrchr(full_path, '/');
    if (filename == NULL) {
        fprintf(stderr, "Invalid file path: %s\n", filepath);
        return -1;
    }
    *filename = '\0'; // 将路径和文件名分割开
    filename++; // 指向文件名部分
    dir_path = full_path;
    printf("dir path : %s\n",dir_path);

    // 检查是否在陀螺仪触发目录下
    if (strncmp(dir_path, GYRO_TRIGGER_PHOTO_FILE_DIR, strlen(GYRO_TRIGGER_PHOTO_FILE_DIR)-1) == 0) {
        *trigger_type = GYRO_TRIGGER_TYPE;
        unsigned int parsed_id;
        int parsed_angle;
        // 文件名格式：image_id_angle
        if (sscanf(filename, "image_%u_%d", &parsed_id, &parsed_angle) != 2 || 
            parsed_id > UCHAR_MAX || parsed_angle > SCHAR_MAX || parsed_angle < SCHAR_MIN) {
            fprintf(stderr, "Failed to parse filename or value out of byte range: %s\n", filename);
            return -1;
        }
        printf("gyro\n");
        *id = (unsigned char)parsed_id;
        *angle = (signed char)parsed_angle;
    }
    // 检查是否在主动拍照目录下
    else if (strncmp(dir_path, ACTIVE_TRIGGER_PHOTO_FILE_DIR, strlen(ACTIVE_TRIGGER_PHOTO_FILE_DIR)-1) == 0) {
        *trigger_type = ACTIVE_TRIGGER_TYPE;
        unsigned int parsed_id;
        // 文件名格式：image_id
        if (sscanf(filename, "image_%u", &parsed_id) != 1 || parsed_id > UCHAR_MAX) {
            fprintf(stderr, "Failed to parse filename or id out of byte range: %s\n", filename);
            return -1;
        }
        printf("act\n");
        *id = (unsigned char)parsed_id;
        *angle = -1; // 对于主动拍照目录下的文件，angle设置为-1
    } else {
        fprintf(stderr, "Unknown directory path: %s\n", dir_path);
        *trigger_type = TRIGGER_TYPE_UNKNOWN;
        return -1;
    }

    return 0;
}

// 根据文件路径生成图片信息
void generate_image_info(char *image_path) {
    struct stat file_stat;
    if (stat(image_path, &file_stat) != 0) {
        perror("can not get file stat");
        return;
    }

    int ret = 0;
    // 获取文件大小
    unsigned int image_length = file_stat.st_size;

    // 获取抓取时间（文件修改时间）
    unsigned int capture_time = file_stat.st_mtime;

    // 计算MD5哈希值
    unsigned char md5[33] = {0};
    calculate_file_md5(image_path, md5);

    // 解析文件名以获取触发类型和触发角度
    unsigned char trigger_type = 0;
    unsigned char trigger_angle = 0;
    unsigned char pic_id;
    ret = parse_filename(image_path, &trigger_type, &pic_id,&trigger_angle);
    if(ret == 0 )
    {
        LOGD("parse file scuess\n");
    }
    else{
        LOGD("parse file fail\n");
    }

    printf("trigger_type : %d\n",trigger_type);
    printf("pic_id : %d\n",pic_id);
    printf("trigger_angle : %d\n",trigger_angle);
    // // 构建DataNode结构体
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
    printf("trigger_type : %d\n",trigger_type);
    // 根据触发类型决定添加到哪个链表
    if (trigger_type ==GYRO_TRIGGER_TYPE ) 
    {
        appendToGyroscopeTriggerList(data);
    } 
    else if (trigger_type == ACTIVE_TRIGGER_TYPE) 
    {
        appendToActiveTriggerList(data);
    } 
    else 
    {
        printf("unkow tigerr type : %d\n", trigger_type);
    }
    // printList(gyroscopeTriggerList);
    // printList(activeTriggerList);
}