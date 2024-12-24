#include "circular_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <sys/stat.h>

static FILE* log_file = NULL;
static char* log_filename = NULL;
static size_t max_log_size = 0;

// 获取日志级别对应的颜色
static const char* get_level_color(LogLevel level) {
    switch (level) {
        case LOG_DEBUG: return LOG_COLOR_BLUE;
        case LOG_INFO:  return LOG_COLOR_GREEN;
        case LOG_WARN:  return LOG_COLOR_YELLOW;
        case LOG_ERROR: return LOG_COLOR_RED;
        default:        return LOG_COLOR_NONE;
    }
}

int log_init(const char* filename, size_t max_size) {
    log_filename = strdup(filename);
    max_log_size = max_size;
    log_file = fopen(filename, "a+");
    if (!log_file) {
        return -1;
    }
    return 0;
}

void log_write(LogLevel level, const char* tag, const char* format, ...) {
    // 获取时间
    time_t now;
    time(&now);
    char time_str[32];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now));

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
    va_list args1, args2;
    va_start(args1, format);
    va_copy(args2, args1);

    // 写入控制台（带颜色）
    const char* color = get_level_color(level);
    printf("%s[%s][%s][%s] ", color, time_str, level_str, tag);
    vprintf(format, args1);
    printf("%s\n", LOG_COLOR_NONE);
    
    // 写入文件（如果文件已打开）
    if (log_file) {
        // 检查文件大小
        struct stat st;
        stat(log_filename, &st);
        if (st.st_size >= max_log_size) {
            // 如果超过最大大小，重新创建文件
            fclose(log_file);
            log_file = fopen(log_filename, "w");
        }

        // 写入日志
        fprintf(log_file, "[%s][%s][%s] ", time_str, level_str, tag);
        vfprintf(log_file, format, args2);
        fprintf(log_file, "\n");
        fflush(log_file);
    }

    va_end(args1);
    va_end(args2);
}

void log_deinit(void) {
    if (log_file) {
        fclose(log_file);
        log_file = NULL;
    }
    if (log_filename) {
        free(log_filename);
        log_filename = NULL;
    }
}