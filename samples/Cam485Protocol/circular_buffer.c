#include "circular_buffer.h"
// 缓冲区配置
#define CIRCULAR_BUFFER_SIZE 4096    // 缓冲区总容量
#define CIRCULAR_BUFFER_WARN 3584    // 高水位线警告阈值



// 初始化循环缓冲区
CircularBuffer* circular_buffer_create(size_t capacity) {
    if (capacity == 0) {
        fprintf(stderr, "Invalid buffer capacity\n");
        return NULL;
    }

    CircularBuffer *cb = malloc(sizeof(CircularBuffer));
    if (!cb) {
        perror("malloc failed");
        return NULL;
    }

    cb->buffer = malloc(capacity);
    if (!cb->buffer) {
        free(cb);
        perror("buffer malloc failed");
        return NULL;
    }

    pthread_mutex_init(&cb->mutex, NULL);
    pthread_cond_init(&cb->cond_not_empty, NULL);
    pthread_cond_init(&cb->cond_not_full, NULL);

    cb->capacity = capacity;
    cb->head = 0;
    cb->tail = 0;
    cb->count = 0;
    cb->shutdown = false;

    return cb;
}

// 销毁循环缓冲区
void circular_buffer_destroy(CircularBuffer *cb) {
    if (!cb) return;

    pthread_mutex_lock(&cb->mutex);
    cb->shutdown = true;
    pthread_cond_broadcast(&cb->cond_not_empty);
    pthread_cond_broadcast(&cb->cond_not_full);
    pthread_mutex_unlock(&cb->mutex);

    pthread_mutex_destroy(&cb->mutex);
    pthread_cond_destroy(&cb->cond_not_empty);
    pthread_cond_destroy(&cb->cond_not_full);

    free(cb->buffer);
    free(cb);
}

// 获取缓冲区状态
static inline size_t circular_buffer_available(CircularBuffer *cb) {
    return cb->count;
}

static inline size_t circular_buffer_remaining(CircularBuffer *cb) {
    return cb->capacity - cb->count;
}

// 核心写入函数
size_t circular_buffer_write(CircularBuffer *cb, const void *data, size_t bytes, int timeout_ms) {
    if (!cb || !data || bytes == 0) {
        errno = EINVAL;
        return 0;
    }

    pthread_mutex_lock(&cb->mutex);

    size_t bytes_written = 0;
    struct timespec ts;

    // 设置超时时间
    if (timeout_ms > 0) {
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += timeout_ms / 1000;
        long nsec = ts.tv_nsec + (timeout_ms % 1000) * 1000000L;
        ts.tv_sec += nsec / 1000000000L;
        ts.tv_nsec = nsec % 1000000000L;
    }

    while (bytes_written < bytes && !cb->shutdown) {
        // 等待可用空间
        while (circular_buffer_remaining(cb) == 0 && !cb->shutdown) {
            if (timeout_ms == 0) { // 非阻塞模式
                pthread_mutex_unlock(&cb->mutex);
                return bytes_written;
            } else if (timeout_ms > 0) { // 限时等待
                if (pthread_cond_timedwait(&cb->cond_not_full, &cb->mutex, &ts) == ETIMEDOUT) {
                    pthread_mutex_unlock(&cb->mutex);
                    return bytes_written;
                }
            } else { // 无限等待
                pthread_cond_wait(&cb->cond_not_full, &cb->mutex);
            }
        }

        if (cb->shutdown) break;

        // 计算可写入长度
        size_t space_available = circular_buffer_remaining(cb);
        size_t to_write = (bytes - bytes_written < space_available) ? 
                         bytes - bytes_written : space_available;

        // 分两段写入（处理环尾）
        size_t first_chunk = (cb->head + to_write <= cb->capacity) ? 
                           to_write : cb->capacity - cb->head;

        memcpy(cb->buffer + cb->head, (char*)data + bytes_written, first_chunk);
        if (first_chunk < to_write) {
            memcpy(cb->buffer, (char*)data + bytes_written + first_chunk, 
                  to_write - first_chunk);
        }

        cb->head = (cb->head + to_write) % cb->capacity;
        cb->count += to_write;
        bytes_written += to_write;

        // 触发非空条件
        if (bytes_written > 0) {
            pthread_cond_signal(&cb->cond_not_empty);
        }

        // 高水位线警告
        if (cb->count >= CIRCULAR_BUFFER_WARN) {
            fprintf(stderr, "WARNING: Buffer usage exceeds 85%%\n");
        }
    }

    pthread_mutex_unlock(&cb->mutex);
    return bytes_written;
}

// 核心读取函数
size_t circular_buffer_read(CircularBuffer *cb, void *data, size_t bytes, int timeout_ms) {
    if (!cb || !data || bytes == 0) {
        errno = EINVAL;
        return 0;
    }

    pthread_mutex_lock(&cb->mutex);

    size_t bytes_read = 0;
    struct timespec ts;

    // 设置超时时间
    if (timeout_ms > 0) {
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += timeout_ms / 1000;
        long nsec = ts.tv_nsec + (timeout_ms % 1000) * 1000000L;
        ts.tv_sec += nsec / 1000000000L;
        ts.tv_nsec = nsec % 1000000000L;
    }

    while (bytes_read < bytes && !cb->shutdown) {
        // 等待可用数据
        while (circular_buffer_available(cb) == 0 && !cb->shutdown) {
            if (timeout_ms == 0) {
                pthread_mutex_unlock(&cb->mutex);
                return bytes_read;
            } else if (timeout_ms > 0) {
                int ret = pthread_cond_timedwait(&cb->cond_not_empty, &cb->mutex, &ts);
                if (ret == ETIMEDOUT || circular_buffer_available(cb) == 0) {
                    pthread_mutex_unlock(&cb->mutex);
                    return bytes_read;
                }
            } else {
                pthread_cond_wait(&cb->cond_not_empty, &cb->mutex);
                if (cb->shutdown) break;
            }
        }

        if (cb->shutdown) break;

        // 计算可读取长度
        size_t data_available = circular_buffer_available(cb);
        size_t to_read = (bytes - bytes_read < data_available) ?
                        bytes - bytes_read : data_available;

        // 分两段读取（处理环尾）
        size_t first_chunk = (cb->tail + to_read <= cb->capacity) ?
                           to_read : cb->capacity - cb->tail;

        memcpy((char*)data + bytes_read, cb->buffer + cb->tail, first_chunk);
        if (first_chunk < to_read) {
            memcpy((char*)data + bytes_read + first_chunk, cb->buffer,
                  to_read - first_chunk);
        }

        cb->tail = (cb->tail + to_read) % cb->capacity;
        cb->count -= to_read;
        bytes_read += to_read;

        // 触发非满条件
        if (to_read > 0) {
            pthread_cond_broadcast(&cb->cond_not_full);
        }
    }

    pthread_mutex_unlock(&cb->mutex);
    return bytes_read;
}

// 添加一个完整读取的函数
size_t circular_buffer_read_exact(CircularBuffer *cb, void *data, size_t bytes, int timeout_ms) {
    size_t total_read = 0;
    int remaining_timeout = timeout_ms;
    struct timespec start_time;
    
    if (timeout_ms > 0) {
        clock_gettime(CLOCK_REALTIME, &start_time);
    }
    
    while (total_read < bytes) {
        size_t read_size = circular_buffer_read(cb, (char*)data + total_read, 
                                              bytes - total_read, remaining_timeout);
        if (read_size == 0) {
            break;  // 超时或出错
        }
        
        total_read += read_size;
        
        if (timeout_ms > 0 && total_read < bytes) {
            // 更新剩余超时时间
            struct timespec current_time;
            clock_gettime(CLOCK_REALTIME, &current_time);
            long elapsed_ms = (current_time.tv_sec - start_time.tv_sec) * 1000 +
                            (current_time.tv_nsec - start_time.tv_nsec) / 1000000;
            remaining_timeout = timeout_ms - elapsed_ms;
            if (remaining_timeout <= 0) {
                break;
            }
        }
    }
    
    return total_read;
}

/******************** 测试用例 ********************/
#ifdef CIRCULAR_BUFFER_TEST

#include <time.h>
#include <sys/time.h>

// 性能测试参数
#define TEST_ITERATIONS   1000000    // 测试迭代次数
#define BUFFER_SIZE_TEST  4096       // 测试缓冲区大小
#define BLOCK_SIZE        512        // 每次读写块大小

// 生产者线程
void* producer_thread(void *arg) {
    CircularBuffer *cb = (CircularBuffer*)arg;
    unsigned char data[BLOCK_SIZE];
    size_t total = 0;

    memset(data, 0xAA, sizeof(data));

    for (int i = 0; i < TEST_ITERATIONS; ++i) {
        size_t written = circular_buffer_write(cb, data, sizeof(data), -1);
        total += written;
    }

    printf("Producer wrote %zu bytes\n", total);
    return NULL;
}

// 消费者线程
void* consumer_thread(void *arg) {
    CircularBuffer *cb = (CircularBuffer*)arg;
    unsigned char data[BLOCK_SIZE];
    size_t total = 0;

    while (total < TEST_ITERATIONS * BLOCK_SIZE) {
        size_t read = circular_buffer_read(cb, data, sizeof(data), -1);
        total += read;
    }

    printf("Consumer read %zu bytes\n", total);
    return NULL;
}

int main() {
    CircularBuffer *cb = circular_buffer_create(BUFFER_SIZE_TEST);
    if (!cb) return EXIT_FAILURE;

    pthread_t producer, consumer;

    struct timeval start, end;
    gettimeofday(&start, NULL);

    pthread_create(&producer, NULL, producer_thread, cb);
    pthread_create(&consumer, NULL, consumer_thread, cb);

    pthread_join(producer, NULL);
    pthread_join(consumer, NULL);

    gettimeofday(&end, NULL);
    long seconds = end.tv_sec - start.tv_sec;
    long micros = ((seconds * 1000000) + end.tv_usec) - start.tv_usec;

    printf("Throughput: %.2f MB/s\n", 
          (TEST_ITERATIONS * BLOCK_SIZE) / (double)micros);

    circular_buffer_destroy(cb);
    return EXIT_SUCCESS;
}
#endif