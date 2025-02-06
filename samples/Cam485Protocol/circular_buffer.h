#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

typedef struct {
    pthread_mutex_t mutex;           // 互斥锁
    pthread_cond_t cond_not_empty;   // 非空条件变量
    pthread_cond_t cond_not_full;    // 非满条件变量
    
    unsigned char *buffer;           // 数据存储区
    size_t capacity;                 // 缓冲区总容量
    size_t head;                     // 写入位置
    size_t tail;                     // 读取位置
    size_t count;                    // 当前数据量
    bool shutdown;                   // 关闭标志
} CircularBuffer;



CircularBuffer* circular_buffer_create(size_t capacity);
void circular_buffer_destroy(CircularBuffer *cb);
size_t circular_buffer_write(CircularBuffer *cb, const void *data, size_t bytes, int timeout_ms);
size_t circular_buffer_read(CircularBuffer *cb, void *data, size_t bytes, int timeout_ms);
#endif // CIRCULAR_BUFFER_H