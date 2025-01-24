#ifndef T23_SELF_CHECK_H
#define T23_SELF_CHECK_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
// 错误码定义
#define YQSL_ERROR_NAME_OK         "ok"
#define YQSL_ERROR_NAME_NO_MEM     "no_memory"
#define YQSL_ERROR_NAME_OPEN_FAIL  "open_failed"
#define YQSL_ERROR_NAME_START_FAIL "start_failed"
#define YQSL_ERROR_NAME_READ_FAIL  "read_failed"
#define YQSL_ERROR_NAME_WRITE_FAIL "write_failed"
#define YQSL_ERROR_NAME_IMAGE_SIZE "image_size_error"
#define YQSL_ERROR_NAME_TIMEOUT    "timeout"

#define MAX_ERROR_STRING_LENGTH 32

// 自检属性结构体
typedef struct {
    uint64_t startTime;
    uint64_t endTime;
    uint64_t tickStart;
    uint64_t tickEnd;
    char result;
    char resultString[MAX_ERROR_STRING_LENGTH];  // 改用C风格字符串
} SelfCheckProperty;

void self_check_start(void);
SelfCheckProperty* self_check_get_cam_check_property(void);
SelfCheckProperty* self_check_get_gyro_check_property(void);
int self_check_get_status(void);
#endif // T23_SELF_CHECK_H