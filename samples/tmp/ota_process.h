
#ifndef OTA_PROCESS_H
#define OTA_PROCESS_H

#define OTA_DEBUG 0

#define LOGD(fmt, ...)                          \
    do                                          \
    {                                           \
        printf("[%s:%d] ", __func__, __LINE__); \
        printf(fmt, ##__VA_ARGS__);             \
    } while (0)


#define USING_OTA_MESSAGE_QUEUE 0
#define USING_OTA_AUTO_REBOOT 0
#if USING_OTA_MESSAGE_QUEUE
#define OTA_RECV_BUFFER_SIZE     5000
#define OTA_SEND_BUFFER_SIZE     5000
void register_ota_message_queue();
#include "modbus_task.h"
#endif

#define OTA_RESPONSE_OK     1
#define OTA_RESPONSE_ERROR  0

#define OTA_TMP_FILE_PATH "/tmp/ota_app/sample_camera_rst.xz"
#define OTA_TMP_FILE_PATH_UNZIP "/tmp/ota_app/sample_camera_rst"
#define OTA_SYSTEM_APP_EXEC_PATH "/system/bin/sample_camera_rst"

// OTA 消息定义
#define OTA_MSG_READY              "Ready for OTA update"
#define OTA_MSG_INVALID_PARAMS     "Invalid parameters"
#define OTA_MSG_NOT_STARTED        "OTA not started"
#define OTA_MSG_JSON_PARSE_ERROR   "JSON parse error"
#define OTA_MSG_SIZE_EXCEED        "Data size exceeds expected size"
#define OTA_MSG_FILE_OPEN_ERROR    "File open error" 
#define OTA_MSG_FILE_WRITE_ERROR   "File write error"
#define OTA_MSG_MD5_CALC_FAILED    "MD5 calculation failed"
#define OTA_MSG_MD5_VERIFY_FAILED  "MD5 verification failed"
#define OTA_MSG_MD5_VERIFIED       "File received completely, MD5 verified"
#define OTA_MSG_DECOMPRESS_FAILED  "Decompress OTA file failed"
#define OTA_MSG_COPY_FAILED        "Copy OTA file to system app exec path failed"
#define OTA_MSG_FILE_INCOMPLETE    "File not completely received"
#define OTA_MSG_UPDATE_SUCCESS     "OTA completed successfully"
#define OTA_MSG_DIR_CREATE_ERROR   "Create directory failed"
#define OTA_MSG_FILE_PERMISSION_ERROR "File permission error"
// 进度消息格式
#define OTA_MSG_PROGRESS_FORMAT    "Data received, progress: %d%%"





enum OTA_REBOOT_FLAG
{
    OTA_REBOOT_FLAG_NONE = 0,
    OTA_REBOOT_FLAG_REBOOT = 1,
};
int ota_process(char *res, int res_len, char *data, int len);
#endif

