#ifndef CIRCULAR_LOG_H
#define CIRCULAR_LOG_H

#include <stdio.h>

// 定义日志级别
typedef enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR
} LogLevel;

// 定义日志颜色（用于控制台输出）
#define LOG_COLOR_NONE         "\033[0m"
#define LOG_COLOR_RED         "\033[0;31m"
#define LOG_COLOR_GREEN       "\033[0;32m"
#define LOG_COLOR_YELLOW      "\033[0;33m"
#define LOG_COLOR_BLUE        "\033[0;34m"

// 初始化日志系统
int log_init(const char* filename, size_t max_size);

// 写入日志
void log_write(LogLevel level, const char* tag, const char* format, ...);

// 清理日志系统
void log_deinit(void);

#endif // CIRCULAR_LOG_H