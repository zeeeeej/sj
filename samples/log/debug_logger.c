#include "debug_logger.h"
#if DEBUG_LOG_ENABLE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <pthread.h>

#define MAX_LOGGERS 10
#define TAG_LENGTH 32



typedef struct {
    pthread_t thread;
    int logging_enabled;
    int stop_thread;
    char flag_file_path[256];
    char tag[TAG_LENGTH];
    DebugLogLevel level; // 添加日志等级字段
} DebugLogger;

static DebugLogger loggers[MAX_LOGGERS];
static int logger_count = 0;

// 获取日志类别的字符串表示
const char* get_log_level_string(DebugLogLevel level) {
    switch (level) {
        case DEBUG_LOG_INFO: return "INFO";
        case DEBUG_LOG_DEBUG: return "DEBUG";
        case DEBUG_LOG_WARN: return "WARN";
        case DEBUG_LOG_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

// 调试线程函数
void *debug_thread_function(void *arg) {
    DebugLogger *logger = (DebugLogger *)arg;
    while (!logger->stop_thread) {
        if (access(logger->flag_file_path, F_OK) == 0) {
            logger->logging_enabled = 1;
        } else {
            logger->logging_enabled = 0;
        }
        sleep(1); // 每秒检查一次
    }
    return NULL;
}

// 注册调试日志实例
int register_debug_logger(const char *tag, const char *flag_file_path) {
    if (logger_count >= MAX_LOGGERS) {
        fprintf(stderr, "Error: Maximum number of loggers reached\n");
        return -1;
    }

    DebugLogger *logger = &loggers[logger_count++];
    strncpy(logger->tag, tag, TAG_LENGTH - 1);
    logger->tag[TAG_LENGTH - 1] = '\0';
    strncpy(logger->flag_file_path, flag_file_path, sizeof(logger->flag_file_path) - 1);
    logger->flag_file_path[sizeof(logger->flag_file_path) - 1] = '\0';
    logger->logging_enabled = 0;
    logger->stop_thread = 0;
    logger->level = DEBUG_LOG_INFO; // 默认日志等级

    int ret = pthread_create(&logger->thread, NULL, debug_thread_function, logger);
    if (ret != 0) {
        fprintf(stderr, "Error: Unable to create debug thread: %s\n", strerror(ret));
        return -1;
    }
    return 0;
}

// 注销调试日志实例
void unregister_debug_logger(const char *tag) {
    for (int i = 0; i < logger_count; ++i) {
        if (strcmp(loggers[i].tag, tag) == 0) {
            loggers[i].stop_thread = 1;
            pthread_join(loggers[i].thread, NULL);
            // 移除logger
            for (int j = i; j < logger_count - 1; ++j) {
                loggers[j] = loggers[j + 1];
            }
            --logger_count;
            break;
        }
    }
}

// 设置日志等级
void set_log_level(const char *tag, DebugLogLevel level) {
    for (int i = 0; i < logger_count; ++i) {
        if (strcmp(loggers[i].tag, tag) == 0) {
            loggers[i].level = level;
            break;
        }
    }
}

void debug_log(const char *tag, DebugLogLevel level, const char *format, ...) {
    for (int i = 0; i < logger_count; ++i) {
        if (strcmp(loggers[i].tag, tag) == 0 && loggers[i].logging_enabled) {
            if (level >= loggers[i].level) { // 仅当日志等级足够高时才打印
                va_list args;
                va_start(args, format);
                vprintf(format, args); // 直接打印格式化的日志信息
                va_end(args);
            }
            break;
        }
    }
}

#define MPU_INFO_TAG "mpu_info"
#define MPU_INFO_FLAG_FILE "/system/debug/mpu_info"

#define MPU_DEBUG_TAG "mpu_debug"
#define MPU_DEBUG_FLAG_FILE "/system/debug/mpu_debug"

#define WIND_INFO_TAG "wind_info"
#define WIND_INFO_FLAG_FILE "/system/debug/wind_info"

#define WIND_DEBUG_TAG "wind_debug"
#define WIND_DEBUG_FLAG_FILE "/system/debug/wind_debug"

// 初始化调试日志
int debug_logger_init()
{
    if (register_debug_logger(MPU_INFO_TAG, MPU_INFO_FLAG_FILE) != 0) {
        fprintf(stderr, "Failed to register mpu info logger\n");
        return -1;
    }
    if (register_debug_logger(MPU_DEBUG_TAG, MPU_DEBUG_FLAG_FILE) != 0) {
        fprintf(stderr, "Failed to register mpu debug logger\n");
        return -1;
    }
    if (register_debug_logger(WIND_INFO_TAG, WIND_INFO_FLAG_FILE) != 0) {
        fprintf(stderr, "Failed to register wind info logger\n");
        return -1;
    }
    if (register_debug_logger(WIND_DEBUG_TAG, WIND_DEBUG_FLAG_FILE) != 0) {
        fprintf(stderr, "Failed to register wind debug logger\n");
        return -1;
    }
    return 0;
}

// 注销调试日志
void debug_logger_deinit()
{
    unregister_debug_logger(MPU_INFO_TAG);
    unregister_debug_logger(MPU_DEBUG_TAG);
    unregister_debug_logger(WIND_INFO_TAG);
    unregister_debug_logger(WIND_DEBUG_TAG);
}

#endif