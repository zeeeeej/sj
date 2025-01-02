#ifndef DEBUG_LOGGER_H
#define DEBUG_LOGGER_H

#define DEBUG_LOG_ENABLE 1

#include <pthread.h>

typedef enum {
    DEBUG_LOG_INFO,
    DEBUG_LOG_DEBUG,
    DEBUG_LOG_WARN,
    DEBUG_LOG_ERROR
} DebugLogLevel;

// 初始化调试日志实例
int register_debug_logger(const char *tag, const char *flag_file_path);

// 停止调试日志实例
void unregister_debug_logger(const char *tag);

// 打印调试信息
void debug_log(const char *tag, DebugLogLevel level, const char *format, ...);

int debug_logger_init();
void debug_logger_deinit();

#endif // DEBUG_LOGGER_H
