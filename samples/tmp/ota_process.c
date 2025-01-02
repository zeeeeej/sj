/**
 * @file    ota_process.c
 * @author  lijunhong
 * @brief   OTA升级处理模块
 * @version 1.0
 * @date    2024-13-13
 */
#include <pthread.h>
#include <mqueue.h>
#include "stdint.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/reboot.h>
#include <sys/types.h> // for mkdir()
#include <sys/stat.h>  // for struct stat, mkdir()
#include <unistd.h>    // for access()
#include <limits.h>    // for PATH_MAX
#include "ota_process.h"
#include <string.h>
#include "cJSON.h"
#include "cm_base64.h"
#include "circular_log.h"

static char* TAG_NAME = "ota process";

// 添加日志标签定义
#define OTA_TAG "OTA"

// 添加日志宏定义，只保留关键日志级别
#define OTA_LOG_ERROR(fmt, ...) log_write(LOG_ERROR, OTA_TAG, fmt, ##__VA_ARGS__)
#define OTA_LOG_INFO(fmt, ...)  log_write(LOG_INFO, OTA_TAG, fmt, ##__VA_ARGS__)

#ifdef DEBUG  // 在调试模式下才启用 DEBUG 日志
#define OTA_LOG_DEBUG(fmt, ...) log_write(LOG_DEBUG, OTA_TAG, fmt, ##__VA_ARGS__)
#else
#define OTA_LOG_DEBUG(fmt, ...) ((void)0)
#endif


// 通用帧头格式:
// {
//     "timestamp": uint64_t,  // 时间戳
//     "seqNum": uint32_t,     // 序列号
//     "cmdType": uint32_t,    // 命令类型
//     "result": int           // 结果
// }

// 2. OTA请求帧 (OTA_TYPE_REQUEST):
// {
//     // 通用帧头字段...
//     "version": string,      // 版本号(32字节)
//     "file_size": uint32_t,  // 文件大小
//     "file_md5": string,     // MD5值(32字节)
//     "file_crc": uint32_t,   // CRC校验值(可选)
//     "file_type": uint32_t   // 文件类型(可选)
// }
/**
 * @brief OTA文件信息结构体
 */
typedef struct
{
    char version[32];   // 版本号
    uint32_t file_size; // 文件大小
    char file_md5[33];  // 文件MD5值，32字节+'\0'
    uint32_t file_crc;  // 文件CRC校验值（可选）
    uint32_t file_type; // 文件类型（可选）
} OtaFileInfo;
// 添加全局变量来跟踪升级进度
typedef struct
{
    uint32_t total_size;    // 总大小
    uint32_t received_size; // 已接收大小
    uint8_t is_updating;    // 是否正在升级
    char version[32];       // 目标版本
    char md5[33];           // 期望的MD5值
} ota_status_t;

typedef struct
{
    uint64_t timestamp;
    uint32_t seqNum;
    uint32_t cmdType;
    int result;
} msg_header;
#if USING_OTA_MESSAGE_QUEUE
#define QUEUE_NAME "/ota_queue"
#define MAX_MSG_SIZE 1024
#define QUEUE_PERMISSIONS 0660
static mqd_t recv_que;
static mqd_t send_que;
static uint8_t ota_recv_buffer[OTA_RECV_BUFFER_SIZE];
static uint8_t ota_send_buffer[OTA_SEND_BUFFER_SIZE];
#endif

#if USING_OTA_AUTO_REBOOT
static bool reboot_flag = false;
static pthread_mutex_t reboot_flag_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif
static ota_status_t g_ota_status = {0};

// TODO: 发送验证回复包后，是自己重启，还是等待esp32发送重启命令
// TODO:
// TODO:
// TODO:

typedef enum
{
    /*ota升级请求*/
    OTA_TYPE_REQUEST,
    /*ota升级包数据传输请求*/
    OTA_TYPE_DATA,
    /*ota升级结果请求查询*/
    OTA_TYPE_VERIFY,
    OTA_TYPE_REBOOT,
} ota_sub_type;

/**
 * @brief 解析子命令
 * @param data 接收到的数据
 * @param len 数据长度
 * @param sub_type 解析后的子命令类型
 * @return 0:成功 -1:失败
 */
int parse_sub_command(char *data, int len, ota_sub_type *sub_type)
{
    if (!data || !sub_type || len <= 0)
    {
        LOGD("Invalid parameters for subcommand parsing");
        return -1;
    }

    cJSON *root = cJSON_ParseWithLength(data, len);
    if (!root)
    {
        LOGD("JSON parse failed for subcommand");
        return -1;
    }

    cJSON *subType = cJSON_GetObjectItem(root, "subType");
    if (!subType || !cJSON_IsNumber(subType))
    {
        LOGD("Missing or invalid subType field");
        cJSON_Delete(root);
        return -1;
    }

    *sub_type = (ota_sub_type)subType->valueint;
    cJSON_Delete(root);
    return 0;
}

/**
 * @brief 解析Modbus消息头部
 * @param data 接收到的数据
 * @param len 数长度
 * @param header 解析后的头部结构
 * @return 0:成功 -1:失败
 */
int parse_msg_header(char *data, int len, msg_header *header)
{
    if (!data || !header || len <= 0)
    {
        LOGD("Invalid parameters for header parsing");
        return -1;
    }

    // 解析逻辑
    cJSON *root = cJSON_ParseWithLength(data, len);
    if (!root)
    {
        LOGD("JSON parse failed for header");
        return -1;
    }

    cJSON *timestamp = cJSON_GetObjectItem(root, "timestamp");
    cJSON *seqNum = cJSON_GetObjectItem(root, "seqNum");
    cJSON *cmdType = cJSON_GetObjectItem(root, "cmdType");

    if (!timestamp || !cJSON_IsNumber(timestamp) ||
        !seqNum || !cJSON_IsNumber(seqNum) ||
        !cmdType || !cJSON_IsNumber(cmdType))
    {
        LOGD("Missing or invalid header fields");
        cJSON_Delete(root);
        return -1;
    }

    header->timestamp = (uint64_t)timestamp->valuedouble;
    header->seqNum = (uint32_t)seqNum->valueint;
    header->cmdType = (uint32_t)cmdType->valueint;
    header->result = 1;

    cJSON_Delete(root);
    return 0;
}
int parse_ota_request(char *data, int len, OtaFileInfo *ota_file_info)
{
    // 检查输入参数的有效性
    if (data == NULL || ota_file_info == NULL || len <= 0)
    {
        LOGD("Invalid input parameters");
        return -1;
    }

    // 解析 JSON 数据
    cJSON *root = cJSON_ParseWithLength(data, len);
    if (root == NULL)
    {
        LOGD("Failed to parse JSON");
        return -1;
    }
    // 获取 JSON 对象中的字段
    cJSON *version = cJSON_GetObjectItem(root, "version");
    cJSON *file_size = cJSON_GetObjectItem(root, "file_size");
    cJSON *file_md5 = cJSON_GetObjectItem(root, "file_md5");
    cJSON *file_crc = cJSON_GetObjectItem(root, "file_crc");
    cJSON *file_type = cJSON_GetObjectItem(root, "file_type");

    // 检查必需字段的存在和类型
    if (!version || !file_size || !file_md5 ||
        !cJSON_IsString(version) ||
        !cJSON_IsNumber(file_size) ||
        !cJSON_IsString(file_md5))
    {
        LOGD("Missing or invalid fields");
        cJSON_Delete(root);
        return -1;
    }

    // 将字段值存储到 ota_file_info 结构体中
    strncpy(ota_file_info->version, version->valuestring, sizeof(ota_file_info->version) - 1);
    ota_file_info->file_size = file_size->valueint;
    strncpy(ota_file_info->file_md5, file_md5->valuestring, sizeof(ota_file_info->file_md5) - 1);

    // 检查并存储可选字段
    if (file_crc && cJSON_IsNumber(file_crc))
    {
        ota_file_info->file_crc = file_crc->valueint;
    }

    if (file_type && cJSON_IsNumber(file_type))
    {
        ota_file_info->file_type = file_type->valueint;
    }

    // 释放 JSON 对象
    cJSON_Delete(root);
    return 0;
}
// 打印 OtaFileInfo 结构体中的文件信息
static void print_ota_file_info(const OtaFileInfo *ota_file_info)
{
    if (!ota_file_info)
    {
        return;
    }

    LOGD("OTA Request Info:");
    LOGD("Version: %s", ota_file_info->version);
    LOGD("File Size: %u", ota_file_info->file_size);
    LOGD("File MD5: %s", ota_file_info->file_md5);
    if (ota_file_info->file_crc)
    {
        LOGD("File CRC: %u", ota_file_info->file_crc);
    }
    if (ota_file_info->file_type)
    {
        LOGD("File Type: %u", ota_file_info->file_type);
    }
}

/**
 * @brief 解析 OTA 数据包中的 JSON 数据
 * @param data JSON 数据
 * @param len 数据长度
 * @param decoded_data 解码后的数据
 * @return 成功返回解码数据长度，失败返回-1
 */
static int parse_ota_data_json(char *data, int len, uint8_t **decoded_data)
{
    if (!data || len <= 0 || !decoded_data)
    {
        LOGD("Invalid parameters for JSON parsing");
        return -1;
    }

    // 解析 JSON 数据
    cJSON *root = cJSON_ParseWithLength(data, len);
    if (!root)
    {
        LOGD("Failed to parse JSON");
        return -1;
    }

    // 获取 info 对象
    cJSON *info = cJSON_GetObjectItem(root, "info");
    if (!info || !cJSON_IsObject(info))
    {
        LOGD("Missing or invalid info object");
        cJSON_Delete(root);
        return -1;
    }

    // 获取并解码 base64 数据
    cJSON *data_item = cJSON_GetObjectItem(info, "data");
    if (!data_item || !cJSON_IsString(data_item))
    {
        LOGD("Missing or invalid data field in info object");
        cJSON_Delete(root);
        return -1;
    }

    // 获取数据长度
    cJSON *length_item = cJSON_GetObjectItem(info, "length");
    if (!length_item || !cJSON_IsNumber(length_item))
    {
        LOGD("Missing or invalid length field");
        cJSON_Delete(root);
        return -1;
    }
    uint32_t expected_length = (uint32_t)length_item->valuedouble;
    LOGD("expected_length [%d]\n", expected_length);
    // 计算解码后的数据长度并分配内存
    size_t decoded_len = expected_length*2;
    
    *decoded_data = malloc(decoded_len);
    if (!*decoded_data)
    {
        LOGD("Memory allocation failed for size: %u", expected_length);
        cJSON_Delete(root);
        return -1;
    }
    int ret;

    ret = cm_base64_decode(*decoded_data, &decoded_len,
                         (unsigned char *)data_item->valuestring,
                         strlen(data_item->valuestring));
    if (ret != 0)
    {
        printf("cm_base64_decode ret [%d]\n", ret);
        LOGD("Base64 decode failed");
        free(*decoded_data);
        *decoded_data = NULL;
        cJSON_Delete(root);
        return -1;
    }

    // 验证解码后的长度
    if (decoded_len != expected_length)
    {
        LOGD("Decoded length (%u) doesn't match expected length (%u)",
             decoded_len, expected_length);
        free(*decoded_data);
        *decoded_data = NULL;
        cJSON_Delete(root);
        return -1;
    }

    LOGD("Successfully decoded data, length: %u", decoded_len);
    cJSON_Delete(root);
    return decoded_len;
}

/**
 * @brief 生成 OTA 响应
 * @param res 响应buffer
 * @param res_len buffer长度
 * @param header 消息头
 * @param status 响应状态
 * @param msg 响应消息
 * @return 成功返回响应长度，失败返回-1
 */
static int generate_ota_response(char *res, int res_len, msg_header *header, int status, const char *msg)
{
    if (!res || !msg || res_len <= 0)
    {
        LOGD("Invalid parameters");
        return -1;
    }

    // 创建根对象
    cJSON *root = cJSON_CreateObject();
    if (!root)
    {
        LOGD("Failed to create JSON root object");
        return -1;
    }

    // 添加基本字段
    cJSON_AddNumberToObject(root, "timestamp", header->timestamp);
    cJSON_AddNumberToObject(root, "seqNum", header->seqNum);
    cJSON_AddNumberToObject(root, "cmdType", header->cmdType);
    cJSON_AddNumberToObject(root, "result", status == OTA_RESPONSE_OK ? 1 : 0);
    // 添加消息
    cJSON_AddStringToObject(root, "msg", msg);

    // 转换为字符串
    char *json_str = cJSON_PrintUnformatted(root);
    if (!json_str)
    {
        LOGD("Failed to print JSON string");
        cJSON_Delete(root);
        return -1;
    }

    // 复制到响应buffer
    size_t json_len = strlen(json_str);
    if (json_len >= res_len)
    {
        LOGD("Response buffer too small");
        cJSON_free(json_str);
        cJSON_Delete(root);
        return -1;
    }

    strncpy(res, json_str, res_len - 1);
    res[res_len - 1] = '\0'; // 确保字符串结束

    // 清理资源
    cJSON_free(json_str);
    cJSON_Delete(root);

    LOGD("Generated OTA response: %s", res);
    return strlen(res);
}

static int handle_ota_request(OtaFileInfo *ota_info, char *res, int res_len, msg_header *header)
{
    OTA_LOG_INFO("Handle OTA request");
    if (!ota_info || !res || !header)
    {
        OTA_LOG_ERROR("Invalid parameters");
        return -1;
    }
    // OTA_LOG_INFO("Handle OTA request: version=%s, size=%u", ota_info->version, ota_info->file_size);

    // 初始化升级状态
    memset(&g_ota_status, 0, sizeof(g_ota_status));
    g_ota_status.total_size = ota_info->file_size;
    g_ota_status.is_updating = 1;
    strncpy(g_ota_status.version, ota_info->version, sizeof(g_ota_status.version) - 1);
    strncpy(g_ota_status.md5, ota_info->file_md5, sizeof(g_ota_status.md5) - 1);

    // 删除可能存在的旧文件
    remove(OTA_TMP_FILE_PATH);

    return generate_ota_response(res, res_len, header, OTA_RESPONSE_OK, "Ready for OTA update");
}

/**
 * @brief 生成带进度信息的 OTA 响应
 * @param res 响应buffer
 * @param res_len buffer长度
 * @param header 消息头
 * @param status 响应状态
 * @param msg 响应消息
 * @param progress 当前进度(0-100)
 * @return 成功返回响应长度，失败返回-1
 */
static int generate_ota_response_with_progress(char *res, int res_len, msg_header *header,
                                               int status, const char *msg, int progress)
{
    if (!res || !msg || res_len <= 0)
    {
        LOGD("Invalid parameters");
        return -1;
    }

    // 创建根对象
    cJSON *root = cJSON_CreateObject();
    if (!root)
    {
        LOGD("Failed to create JSON root object");
        return -1;
    }

    // 添加基本字段
    cJSON_AddNumberToObject(root, "timestamp", header->timestamp);
    cJSON_AddNumberToObject(root, "seqNum", header->seqNum);
    cJSON_AddNumberToObject(root, "cmdType", header->cmdType);
    cJSON_AddNumberToObject(root, "result", status == OTA_RESPONSE_OK ? 1 : 0);

    // 创建info对象
    cJSON *info = cJSON_CreateObject();
    if (!info)
    {
        LOGD("Failed to create info object");
        cJSON_Delete(root);
        return -1;
    }

    // 添加消息和进度信息
    cJSON_AddStringToObject(info, "msg", msg);
    cJSON_AddNumberToObject(info, "progress", progress);

    // 将info对象添加到根对象
    cJSON_AddItemToObject(root, "info", info);

    // 转换为字符串
    char *json_str = cJSON_PrintUnformatted(root);
    if (!json_str)
    {
        LOGD("Failed to print JSON string");
        cJSON_Delete(root);
        return -1;
    }

    // 复制到响应buffer
    size_t json_len = strlen(json_str);
    if (json_len >= res_len)
    {
        LOGD("Response buffer too small");
        cJSON_free(json_str);
        cJSON_Delete(root);
        return -1;
    }

    strncpy(res, json_str, res_len - 1);
    res[res_len - 1] = '\0'; // 确保字符串束

    // 清理资源
    cJSON_free(json_str);
    cJSON_Delete(root);

    LOGD("Generated OTA response with progress: %s", res);
    return strlen(res);
}

/**
 * @brief 解压OTA文件
 * @param ota_file_path OTA压缩文件路径
 * @param unzip_file_path 解压后文件路径
 * @return 成功返回0，失败返回对应错误码
 */
static int decompress_ota_file(const char *ota_file_path, const char *unzip_file_path)
{
    if (!ota_file_path || !unzip_file_path)
    {
        LOGD("Invalid parameters");
        return -EINVAL;
    }

    // 检查源文件是否存在
    if (access(ota_file_path, F_OK) != 0)
    {
        LOGD("OTA file not found: %s", ota_file_path);
        return -ENOENT;
    }

    // 检查目标路径所在目录是否存在,不存在则创建
    char dir_path[PATH_MAX] = {0};
    strncpy(dir_path, unzip_file_path, sizeof(dir_path) - 1);
    char *last_slash = strrchr(dir_path, '/');
    if (last_slash)
    {
        *last_slash = '\0';
        if (access(dir_path, F_OK) != 0)
        {
            if (mkdir(dir_path, 0755) != 0)
            {
                LOGD("Failed to create directory: %s, error: %s",
                     dir_path, strerror(errno));
                return -errno;
            }
        }
    }

    // 构造解压命令
    char cmd[512] = {0};
    snprintf(cmd, sizeof(cmd), "xz -df %s -c > %s", ota_file_path, unzip_file_path);

    // 执行解压命令
    int ret = system(cmd);
    if (ret != 0)
    {
        LOGD("Failed to decompress file, command: %s, ret: %d", cmd, ret);
        return -1;
    }

    // 验证解压的文件
    if (access(unzip_file_path, F_OK) != 0)
    {
        LOGD("Decompressed file not found: %s", unzip_file_path);
        return -ENOENT;
    }

    // 获取解压后文件大小
    struct stat st;
    if (stat(unzip_file_path, &st) == 0)
    {
        if (st.st_size == 0)
        {
            LOGD("Decompressed file is empty");
            unlink(unzip_file_path); // 删除空文件
            return -ENODATA;
        }
        LOGD("File decompressed successfully, size: %ld bytes", st.st_size);
    }

    return 0;
}

/**
 * @brief 复制文件
 * @param src_path 源文件路径
 * @param dst_path 目标文件路径
 * @return 成功返回0，失败返回错误码
 */
static int copy_file(const char *src_path, const char *dst_path)
{
    if (!src_path || !dst_path)
    {
        LOGD("Invalid parameters");
        return -1;
    }

    FILE *src = NULL;
    FILE *dst = NULL;
    char buf[4096];
    size_t nread;
    int ret = 0;

        // 检查源文件是否存在
    if (access(src_path, F_OK) != 0)
    {
        LOGD("Source file does not exist: %s", src_path);
        return -2;
    }

    // 检查目标文件是否存在，存在则删除
    if (access(dst_path, F_OK) == 0)
    {
        LOGD("Destination file exists, removing: %s", dst_path);
        if (unlink(dst_path) != 0)
        {
            LOGD("Failed to remove existing destination file: %s", strerror(errno));
            return -3;
        }
    }
    // 打开源文件
    src = fopen(src_path, "rb");
    if (!src)
    {
        LOGD("Cannot open source file: %s", src_path);
        return -2;
    }

    // 打开目标文件
    dst = fopen(dst_path, "wb");
    if (!dst)
    {
        LOGD("Cannot open destination file: %s", dst_path);
        fclose(src);
        return -3;
    }

    // 复制数据
    while ((nread = fread(buf, 1, sizeof(buf), src)) > 0)
    {
        size_t written = fwrite(buf, 1, nread, dst);
        if (written != nread)
        {
            LOGD("Write error");
            ret = -4;
            break;
        }
    }

    // 检查是否有读取错误
    if (ferror(src))
    {
        LOGD("Read error");
        ret = -5;
    }

    // 确保数据写入磁盘
    if (fflush(dst) != 0)
    {
        LOGD("Flush error");
        ret = -6;
    }

    // 同步文件系统
    fsync(fileno(dst));

    // 设置可执行权限
    if (ret == 0)
    {
        if (chmod(dst_path, 0755) != 0)
        {
            LOGD("Failed to set executable permission");
            ret = -7;
        }
    }

    // 关闭文件
    fclose(src);
    fclose(dst);

    // 如果发生错误，删除目标文件
    if (ret != 0)
    {
        unlink(dst_path);
    }

    return ret;
}

/**
 * @brief 处理 OTA 数据包
 */
static int handle_ota_data(char *data, int len, char *res, int res_len, msg_header *header)
{
    LOGD("Handle OTA data packet, length: %d", len);
    // OTA_LOG_INFO("Handle OTA data packet, length: %d", len);
    if (!data || !res || !header || len <= 0)
    {
        OTA_LOG_ERROR("Invalid parameters");
        return generate_ota_response(res, res_len, header, OTA_RESPONSE_ERROR, "Invalid parameters");
    }

    if (!g_ota_status.is_updating)
    {
        OTA_LOG_ERROR("OTA not started");
        return generate_ota_response(res, res_len, header, OTA_RESPONSE_ERROR, OTA_MSG_NOT_STARTED);
    }

    // OTA_LOG_INFO("Handle OTA data packet, length: %d", len);

    uint8_t *decoded_data = NULL;
    int decoded_len = parse_ota_data_json(data, len, &decoded_data);
    if (decoded_len < 0)
    {
        return generate_ota_response(res, res_len, header, OTA_RESPONSE_ERROR, OTA_MSG_JSON_PARSE_ERROR);
    }

    // 检查接收的数据量是否会超过预期大小
    if (g_ota_status.received_size + decoded_len > g_ota_status.total_size)
    {
        LOGD("Received data size would exceed expected size. Expected: %u, Current: %u, New: %d",
             g_ota_status.total_size, g_ota_status.received_size, decoded_len);
        free(decoded_data);
        return generate_ota_response(res, res_len, header, OTA_RESPONSE_ERROR, OTA_MSG_SIZE_EXCEED);
    }

    // 写入临时文件
    FILE *file = fopen(OTA_TMP_FILE_PATH, "ab");
    if (!file)
    {
        // 如果文件打开失败，检查是否是因为目录不存在
        char dir_path[PATH_MAX] = {0};
        strncpy(dir_path, OTA_TMP_FILE_PATH, sizeof(dir_path) - 1);
        char *last_slash = strrchr(dir_path, '/');
        if (last_slash)
        {
            *last_slash = '\0';
            // 创建目录(如果不存在)
            if (access(dir_path, F_OK) != 0)
            {
                if (mkdir(dir_path, 0755) != 0)
                {
                    OTA_LOG_ERROR("Failed to create directory: %s, error: %s",
                         dir_path, strerror(errno));
                    free(decoded_data);
                    return generate_ota_response(res, res_len, header,
                                                 OTA_RESPONSE_ERROR,
                                                 OTA_MSG_DIR_CREATE_ERROR);
                }
            }
            // 重试打开文件
            file = fopen(OTA_TMP_FILE_PATH, "ab");
            if (!file)
            {
                OTA_LOG_ERROR("Failed to open file after creating directory: %s",
                     strerror(errno));
                free(decoded_data);
                return generate_ota_response(res, res_len, header,
                                             OTA_RESPONSE_ERROR,
                                             OTA_MSG_FILE_OPEN_ERROR);
            }
        }
    }

    // 设置文件权限
    if (chmod(OTA_TMP_FILE_PATH, 0644) != 0)
    {
        OTA_LOG_ERROR("Failed to set file permissions: %s", strerror(errno));
        // 继续执行，不返回错误
    }

    size_t written = fwrite(decoded_data, 1, decoded_len, file);
    fclose(file);
    free(decoded_data);

    if (written != decoded_len)
    {
        OTA_LOG_ERROR("Failed to write data to file");
        return generate_ota_response(res, res_len, header, OTA_RESPONSE_ERROR, OTA_MSG_FILE_WRITE_ERROR);
    }

    // 更新接收进度
    g_ota_status.received_size += decoded_len;

    // 计算进度百分比
    int progress = (g_ota_status.received_size * 100) / g_ota_status.total_size;
    char msg[64];

    // 检查是否接收完成
    if (g_ota_status.received_size >= g_ota_status.total_size)
    {
        // 计算接收到的文件的 MD5
        char calculated_md5[33];
        if (calculate_file_md5(OTA_TMP_FILE_PATH, calculated_md5) != 0)
        {
            OTA_LOG_ERROR("Failed to calculate MD5");
            return generate_ota_response(res, res_len, header, OTA_RESPONSE_ERROR, OTA_MSG_MD5_CALC_FAILED);
        }

        // 比较 MD5
        if (strcmp(calculated_md5, g_ota_status.md5) != 0)
        {
            OTA_LOG_ERROR("MD5 verification failed. Expected: %s, Got: %s", g_ota_status.md5, calculated_md5);
            snprintf(msg, sizeof(msg), "MD5 verification failed");
            return generate_ota_response(res, res_len, header, OTA_RESPONSE_ERROR, OTA_MSG_MD5_VERIFY_FAILED);
        }

        snprintf(msg, sizeof(msg), "File received completely, MD5 verified");
        OTA_LOG_INFO("File received completely, MD5 verified");
    }
    else
    {
        snprintf(msg, sizeof(msg), "Data received, progress: %d%%", progress);
    }

    return generate_ota_response_with_progress(res, res_len, header, OTA_RESPONSE_OK, msg, progress);
}
/**
 * @brief 处理 OTA 验证请求
 */
static int handle_ota_verify(char *data, int len, char *res, int res_len, msg_header *header)
{
    OTA_LOG_INFO("Verifying OTA package");
    if (!data || !res || !header)
    {
        OTA_LOG_ERROR("Invalid parameters");
        return -1;
    }
    if (!g_ota_status.is_updating)
    {
        OTA_LOG_ERROR("OTA not started");
        return generate_ota_response(res, res_len, header, OTA_RESPONSE_ERROR, OTA_MSG_NOT_STARTED);
    }

    OTA_LOG_INFO("Handle OTA verify request");

    // 检查文件是否接收完整
    if (g_ota_status.received_size != g_ota_status.total_size)
    {
        OTA_LOG_ERROR("File not completely received. Expected: %u, Got: %u",
             g_ota_status.total_size, g_ota_status.received_size);
        return generate_ota_response(res, res_len, header, OTA_RESPONSE_ERROR, OTA_MSG_FILE_INCOMPLETE);
    }

    int ret = decompress_ota_file(OTA_TMP_FILE_PATH, OTA_TMP_FILE_PATH_UNZIP);
    if (ret != 0)
    {
        OTA_LOG_ERROR("Decompress OTA file failed");
        return generate_ota_response(res, res_len, header, OTA_RESPONSE_ERROR, OTA_MSG_DECOMPRESS_FAILED);
    }
    OTA_LOG_INFO("Decompress OTA file success");

    // 复制到系统app执行路径
    ret = copy_file(OTA_TMP_FILE_PATH_UNZIP, OTA_SYSTEM_APP_EXEC_PATH);
    if (ret != 0)
    {
        OTA_LOG_ERROR("Copy OTA file to system app exec path failed");
        return generate_ota_response(res, res_len, header, OTA_RESPONSE_ERROR, OTA_MSG_COPY_FAILED);
    }

    // TODO: 更新启动参数
    // TODO: 准备重启

    // 重置 OTA 状态
    OTA_LOG_INFO("OTA completed successfully");
    return generate_ota_response(res, res_len, header, OTA_RESPONSE_OK, "OTA completed successfully");
}
/**
 * @brief 执行系统重启
 * @return 成功返回0,失败返回-1
 */
static int ota_reboot()
{
    OTA_LOG_INFO("System will reboot now...");

    // 同步文件系统,确保数据写入磁盘
    sync();
    int seconds = 5;
    while (seconds > 0) {
        LOGD("Rebooting in %d seconds...", seconds);
        sleep(1);  // 等待1秒
        seconds--;
    }
    // 设置重启标志
    // set_reboot_flag(OTA_REBOOT_REQUIRED);

    // 使用reboot系统调用重启系统
    if (reboot(RB_AUTOBOOT) < 0)
    {
        OTA_LOG_ERROR("Reboot failed: %s", strerror(errno));
        return -1;
    }

    return 0; // 正常情况下不会执行到这里
}

/**
 * @brief 处理 OTA 重启请求
 * @param data 接收到的数据
 * @param len 数据长度
 * @param res 响应buffer
 * @param res_len buffer长度
 * @param header 消息头
 * @return 成功返回响应长度,失败返回-1
 */
static int handle_ota_reboot(char *data, int len, char *res, int res_len, msg_header *header)
{
    LOGD("Handle OTA reboot request");

    // 检查参数
    if (!res || !header || res_len <= 0)
    {
        LOGD("Invalid parameters");
        return -1;
    }

    // 检查是否完成OTA更新
    // if (!g_ota_status.is_updating)
    // {
    //     return generate_ota_response(res, res_len, header,
    //                                  OTA_RESPONSE_ERROR,
    //                                  "OTA not started or already completed");
    // }

    // 执行重启
    int ret = ota_reboot();
    if (ret != 0)
    {
        return generate_ota_response(res, res_len, header,
                                     OTA_RESPONSE_ERROR,
                                     "Reboot failed");
    }

    // 生成响应(正常情况下不会执行到这里)
    return generate_ota_response(res, res_len, header,
                                 OTA_RESPONSE_OK,
                                 "System rebooting...");
}

int ota_process(char *res, int res_len, char *data, int len)
{
    int ret = 0;
    uint32_t sub_type = 0;
    msg_header header;
    OtaFileInfo ota_file_info;

    /*打印ota数据*/
#if OTA_DEBUG
    LOGD("OTA data: %s", data);
#endif

    // 参数检查
    if (!res || !data || res_len <= 0 || len <= 0)
    {
        LOGD("Invalid parameters");
        return -1;
    }

    // 解析子命令类型
    ret = parse_sub_command(data, len, &sub_type);
    if (ret != 0)
    {
        // LOGD("Parse sub command failed");
        OTA_LOG_ERROR("Parse sub command failed");
        return -1;
    }
    // 解析子命令类型
    ret = parse_msg_header(data, len, &header);
    if (ret != 0)
    {
        // LOGD("Parse msg header failed");
        OTA_LOG_ERROR("Parse msg header failed");
        return -1;
    }

    // 根据子命令类型进行处理
    switch (sub_type)
    {
    case OTA_TYPE_REQUEST: // OTA 请求
        memset(&ota_file_info, 0, sizeof(OtaFileInfo));
        ret = parse_ota_request(data, len, &ota_file_info);
        if (ret != 0)
        {
            // LOGD("Parse OTA request failed");
            OTA_LOG_ERROR("Parse OTA request failed");
            return -1;
        }
        print_ota_file_info(&ota_file_info);
        // 处理 OTA 请求
        ret = handle_ota_request(&ota_file_info, res, res_len, &header);
        break;

    case OTA_TYPE_DATA: // OTA 数据包
        ret = handle_ota_data(data, len, res, res_len, &header);
        break;

    case OTA_TYPE_VERIFY: // OTA 验证
        ret = handle_ota_verify(data, len, res, res_len, &header);
        break;

    case OTA_TYPE_REBOOT: // OTA 重启
        ret = handle_ota_reboot(data, len, res, res_len, &header);
        break;

    default:
        LOGD("Unknown OTA sub command type: %d", sub_type);
        ret = -1;
        break;
    }

    return ret;
}

#if USING_OTA_AUTO_REBOOT
void set_reboot_flag(enum OTA_REBOOT_FLAG flag)
{
    pthread_mutex_lock(&reboot_flag_mutex);
    reboot_flag = flag;
    pthread_mutex_unlock(&reboot_flag_mutex);
}

void get_reboot_flag(bool *flag)
{
    pthread_mutex_lock(&reboot_flag_mutex);
    *flag = reboot_flag;
    pthread_mutex_unlock(&reboot_flag_mutex);
}

#endif

#if USING_OTA_MESSAGE_QUEUE

// 初始化消息队列
mqd_t init_ota_queue()
{
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    mqd_t mq = mq_open(QUEUE_NAME, O_CREAT | O_RDWR, QUEUE_PERMISSIONS, &attr);
    if (mq == (mqd_t)-1)
    {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }
    return mq;
}

/**
 * @brief 从消息队列接收数据并存储到本地缓冲区
 * @param mq 消息队列描述符
 * @return 成功返回接收的数据长度，失败返回-1
 */
int receive_ota_message(mqd_t mq)
{
    QueueMessage msg;
    ssize_t bytes_received = mq_receive(mq, (char *)&msg, sizeof(QueueMessage), NULL);
    if (bytes_received == -1)
    {
        perror("mq_receive");
        return -1;
    }

    if (msg.data && msg.length > 0)
    {
        // Ensure we don't overflow the buffer
        size_t copy_length = msg.length < OTA_RECV_BUFFER_SIZE ? msg.length : OTA_RECV_BUFFER_SIZE;
        memcpy(ota_recv_buffer, msg.data, copy_length);
        LOGD("Received data: %.*s", (int)copy_length, ota_recv_buffer);

        // Immediately free the message data if a free function is provided
        if (msg.free_ptr)
        {
            msg.free_ptr(NULL, msg.data);
        }
    }
    else
    {
        LOGD("Received invalid message");
        return -1;
    }

    return msg.length;
}

void send_buf_free(void *priv, void *data)
{
    free(data);
}

/**
 * @brief 发送数据到消息队列
 * @param mq 消息队列描述符
 * @param length 要发送的数据长度
 * @return 成功返回0，失败返回-1
 */
int send_ota_message(mqd_t mq, int length)
{
    // 参数检查
    if (length <= 0 || length > OTA_SEND_BUFFER_SIZE)
    {
        LOGD("Invalid send length: %d", length);
        return -1;
    }

    // 构造消息
    QueueMessage msg;
    msg.length = length;
    msg.free_ptr = send_buf_free;

    // 分配内存并复制数据
    msg.data = malloc(length);
    if (!msg.data)
    {
        LOGD("Failed to allocate memory for message");
        return -1;
    }
    memcpy(msg.data, ota_send_buffer, length);

    // 发送消息
    if (mq_send(mq, (const char *)&msg, sizeof(QueueMessage), 0) == -1)
    {
        LOGD("Failed to send message to queue");
        free(msg.data);
        return -1;
    }

    return 0;
}

/**
 * @brief 处理队列中的消息
 */
void *ota_process_thread(void *arg)
{
    int ret;
    while (1)
    {
        // 接收消息
        ret = receive_ota_message(recv_que);
        if (ret < 0)
        {
            LOGD("Failed to receive message, continue...");
            continue;
        }

        // 处理消息
        ret = ota_process(ota_send_buffer, sizeof(ota_send_buffer), (char *)ota_recv_buffer, ret);
        if (ret < 0)
        {
            LOGD("Failed to process message");
            continue;
        }

        // 发送响应
        if (send_ota_message(send_que, ret) < 0)
        {
            LOGD("Failed to send response");
        }
    }
    return NULL;
}

/**
 * @brief 初始化OTA处理模块
 * @return 成功返回0，失败返回-1
 */
int init_ota_process(void)
{
    // 初始化接收队列
    recv_que = init_ota_queue();
    if (recv_que == (mqd_t)-1)
    {
        LOGD("Failed to init receive queue");
        return -1;
    }

    // 初始化发送队列
    send_que = init_ota_queue();
    if (send_que == (mqd_t)-1)
    {
        LOGD("Failed to init send queue");
        mq_close(recv_que);
        return -1;
    }

    // 启动处理线程
    pthread_t thread;
    if (pthread_create(&thread, NULL, ota_process_thread, NULL) != 0)
    {
        LOGD("Failed to create process thread");
        mq_close(recv_que);
        mq_close(send_que);
        return -1;
    }
    pthread_detach(thread);

    return 0;
}

void register_ota_message_queue()
{
    register_message_queues(recv_que, send_que, 0x9c, init_ota_process);
}

#endif
