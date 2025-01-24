#include "circular_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <errno.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>  // for ftruncate

#define MAX_LOG_BUFFER 4096

static FILE* log_file = NULL;
static char* log_filename = NULL;
static size_t max_log_size = 0;
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
static LogLevel current_log_level = LOG_DEBUG;
static unsigned long log_sequence = 0;
static struct timeval start_time;

static const char* get_level_color(LogLevel level) {
    switch (level) {
        case LOG_DEBUG: return LOG_COLOR_BLUE;
        case LOG_INFO:  return LOG_COLOR_GREEN;
        case LOG_WARN:  return LOG_COLOR_YELLOW;
        case LOG_ERROR: return LOG_COLOR_RED;
        default:        return LOG_COLOR_NONE;
    }
}

void log_set_level(LogLevel level) {
    pthread_mutex_lock(&log_mutex);
    current_log_level = level;
    pthread_mutex_unlock(&log_mutex);
}

int log_init(const char* filename, size_t max_size) {
    pthread_mutex_lock(&log_mutex);
    
    if (log_file != NULL) {
        pthread_mutex_unlock(&log_mutex);
        return 0;
    }

    if (filename == NULL || max_size == 0) {
        pthread_mutex_unlock(&log_mutex);
        return -1;
    }

    // 记录启动时间
    gettimeofday(&start_time, NULL);
    log_sequence = 0;

    log_filename = strdup(filename);
    if (log_filename == NULL) {
        pthread_mutex_unlock(&log_mutex);
        return -1;
    }

    max_log_size = max_size;
    
    // 打开文件以读写模式
    log_file = fopen(filename, "r+");
    if (!log_file) {
        // 如果文件不存在，则创建
        log_file = fopen(filename, "w+");
        if (!log_file) {
            free(log_filename);
            log_filename = NULL;
            pthread_mutex_unlock(&log_mutex);
            return -1;
        }
    }

    // 写入初始化信息
    fprintf(log_file, "=== [SEQ:%lu] Log initialized, max size: %zu bytes ===\n", 
            log_sequence++, max_log_size);
    fflush(log_file);

    pthread_mutex_unlock(&log_mutex);
    return 0;
}

void log_write(LogLevel level, const char* tag, const char* format, ...) {
    if (level < current_log_level || !format || !tag) {
        return;
    }

    pthread_mutex_lock(&log_mutex);

    if (!log_file) {
        pthread_mutex_unlock(&log_mutex);
        return;
    }

    // 计算运行时间
    struct timeval now;
    gettimeofday(&now, NULL);
    unsigned long runtime_ms = (now.tv_sec - start_time.tv_sec) * 1000 + 
                             (now.tv_usec - start_time.tv_usec) / 1000;
    unsigned long hours = runtime_ms / (3600 * 1000);
    unsigned long minutes = (runtime_ms % (3600 * 1000)) / (60 * 1000);
    unsigned long seconds = (runtime_ms % (60 * 1000)) / 1000;
    unsigned long milliseconds = runtime_ms % 1000;

    // 获取日志级别字符串
    const char* level_str;
    switch (level) {
        case LOG_DEBUG: level_str = "DEBUG"; break;
        case LOG_INFO:  level_str = "INFO"; break;
        case LOG_WARN:  level_str = "WARN"; break;
        case LOG_ERROR: level_str = "ERROR"; break;
        default:        level_str = "UNKNOWN";
    }

    // 准备日志消息
    char log_buffer[MAX_LOG_BUFFER];
    va_list args;
    va_start(args, format);
    
    // 格式化日志前缀
    int prefix_len = snprintf(log_buffer, sizeof(log_buffer),
                            "[SEQ:%lu][%02lu:%02lu:%02lu.%03lu][%s][%s] ",
                            log_sequence++, hours, minutes, seconds, milliseconds, 
                            level_str, tag);
    
    // 格式化日志消息
    vsnprintf(log_buffer + prefix_len, sizeof(log_buffer) - prefix_len, format, args);
    va_end(args);

    int total_len = strlen(log_buffer) + 1;  // +1 for \n

    // 检查文件大小并处理
    fseek(log_file, 0, SEEK_END);
    long current_size = ftell(log_file);
    if (current_size + total_len >= max_log_size) {
        // 重置文件指针到开头
        fseek(log_file, 0, SEEK_SET);
        fprintf(log_file, "=== [SEQ:%lu] Log wrapped around here ===\n", log_sequence - 1);
        fflush(log_file);
    }

    // 写入文件
    fprintf(log_file, "%s\n", log_buffer);
    fflush(log_file);

    // 写入控制台（带颜色）
    const char* color = get_level_color(level);
    printf("%s%s%s\n", color, log_buffer, LOG_COLOR_NONE);

    pthread_mutex_unlock(&log_mutex);
}

void log_deinit(void) {
    pthread_mutex_lock(&log_mutex);
    
    if (log_file) {
        fprintf(log_file, "\n=== [SEQ:%lu] Log system shutdown ===\n", log_sequence);
        fflush(log_file);
        fclose(log_file);
        log_file = NULL;
    }
    if (log_filename) {
        free(log_filename);
        log_filename = NULL;
    }
    
    pthread_mutex_unlock(&log_mutex);
}