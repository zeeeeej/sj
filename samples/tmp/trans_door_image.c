#include "stdio.h"
#include "string.h"
#include "errno.h"
#include "sys/stat.h"
#include "unistd.h"
#include "stdint.h"
#include "cJSON.h"

#define PHOTO_STORAGE_DIR "/tmp/camera"
#define PHOTO_FILE_PATH "/tmp/camera/current.jpg"
#define MAX_PHOTO_BUFFER_SIZE 5000
#define TRANSFER_MODE_BINARY 0
#define TRANSFER_MODE_BASE64 1
#define CURRENT_TRANSFER_MODE TRANSFER_MODE_BASE64 
#include "circular_log.h"
static const char *TAG_NAME = "trans_door_image";



#define LOGD(fmt, ...)                          \
    do                                          \
    {                                           \
        printf("[%s:%d] ", __func__, __LINE__); \
        printf(fmt, ##__VA_ARGS__);             \
    } while (0)

// 基础消息头结构
typedef struct {
    uint64_t timestamp;
    uint32_t seqNum;
    uint32_t cmdType;
    int result;
} MessageHeader;

// 照片请求/响应结构
typedef struct {
    MessageHeader header;
    uint32_t open_count;   // 保持原有字段名
    int angle;            // 保持原有字段名
    uint64_t offset;      // 读取照片的偏移量
    uint64_t length;      // 请求的数据长度
    
    // 响应相关字段
    uint64_t size;        // 照片总大小
    uint8_t  data[MAX_PHOTO_BUFFER_SIZE]; // 照片数据
    int data_len;        // 实际读取的数据长度
} PhotoRequest;

static int parse_photo_request(const char *json_data, int len, PhotoRequest *photo)
{
    if (!json_data || !photo || len <= 0) {
        printf("Invalid input parameters\n");
        return -1;
    }

    cJSON *root = cJSON_ParseWithLength(json_data, len);
    if (!root) {
        printf("Failed to parse JSON: %s\n", json_data);
        return -1;
    }

    // 解析时间戳和序列号等基本信息
    cJSON *item = cJSON_GetObjectItem(root, "timestamp");
    if (item && cJSON_IsNumber(item)) {
        photo->header.timestamp = (uint64_t)cJSON_GetNumberValue(item);
        LOGD("timestamp: %llu\n", photo->header.timestamp);
    }

    item = cJSON_GetObjectItem(root, "seqNum");
    if (item && cJSON_IsNumber(item)) {
        photo->header.seqNum = (uint32_t)cJSON_GetNumberValue(item);
        LOGD("seqNum: %u\n", photo->header.seqNum);
    }

    item = cJSON_GetObjectItem(root, "cmdType");
    if (item && cJSON_IsNumber(item)) {
        photo->header.cmdType = (uint32_t)cJSON_GetNumberValue(item);
        LOGD("cmdType: %u\n", photo->header.cmdType);
    }

    // 解析info对象
    cJSON *info = cJSON_GetObjectItem(root, "info");
    if (!info) {
        printf("Missing 'info' object in JSON\n");
        cJSON_Delete(root);
        return -1;
    }

    item = cJSON_GetObjectItem(info, "open_count");
    if (item && cJSON_IsNumber(item)) {
        photo->open_count = (uint32_t)cJSON_GetNumberValue(item);
        LOGD("open_count: %u\n", photo->open_count);
    }

    item = cJSON_GetObjectItem(info, "length");
    if (item && cJSON_IsNumber(item)) {
        photo->length = (uint64_t)cJSON_GetNumberValue(item);
        LOGD("length: %llu\n", photo->length);
    }

    item = cJSON_GetObjectItem(info, "angle");
    if (item && cJSON_IsNumber(item)) {
        photo->angle = (int)cJSON_GetNumberValue(item);
        LOGD("angle: %d\n", photo->angle);
    }

    item = cJSON_GetObjectItem(info, "offset");
    if (item && cJSON_IsNumber(item)) {
        photo->offset = (uint64_t)cJSON_GetNumberValue(item);
        LOGD("offset: %llu\n", photo->offset);
    }

    cJSON_Delete(root);
    return 0;
}
static int generate_photo_response(char *response, int max_len, const PhotoRequest *photo) 
{
    if (!response || !photo || max_len <= 0) {
        LOGD("Invalid input parameters\n");
        return -1;
    }

    cJSON *root = cJSON_CreateObject();
    if (!root) {
        LOGD("Failed to create JSON object\n");
        return -1;
    }

    // 添加基本信息
    cJSON_AddNumberToObject(root, "timestamp", photo->header.timestamp);
    cJSON_AddNumberToObject(root, "seqNum", photo->header.seqNum);
    cJSON_AddNumberToObject(root, "cmdType", photo->header.cmdType);
    
    cJSON *result = cJSON_CreateObject();
    if (!result) {
        // LOGD("Failed to create result object\n");
        log_write(LOG_ERROR, TAG_NAME, "Failed to create result object\n");
        cJSON_Delete(root);
        return -1;
    }

    // 添加结果信息
    cJSON_AddNumberToObject(result, "error", photo->header.result);
    cJSON_AddNumberToObject(result, "size", photo->size);
    cJSON_AddNumberToObject(result, "mode", CURRENT_TRANSFER_MODE);

    #if CURRENT_TRANSFER_MODE == TRANSFER_MODE_BASE64
    {
        // Base64模式
        char *base64_buffer = malloc(photo->data_len * 3);  // 预留足够的Base64编码空间
        if (!base64_buffer) {
            // LOGD("Failed to allocate memory for base64 buffer\n");
            log_write(LOG_ERROR, TAG_NAME, "Failed to allocate memory for base64 buffer\n");
            cJSON_Delete(root);
            return -1;
        }

        size_t base64_len = photo->data_len * 3;
        if (cm_base64_encode((unsigned char *)base64_buffer, &base64_len, 
                            photo->data, photo->data_len) != 0) {
            // LOGD("Failed to encode photo data\n");
            log_write(LOG_ERROR, TAG_NAME, "Failed to encode photo data\n");
            free(base64_buffer);
            cJSON_Delete(root);
            return -1;
        }
        // 打印编码后的长度和原始长度
        LOGD("Base64 encoding successful: original length = %d, encoded length = %zu\n", 
             photo->data_len, base64_len);
        cJSON_AddStringToObject(result, "data", base64_buffer);
        cJSON_AddNumberToObject(result, "length", base64_len);
        
        cJSON_AddItemToObject(root, "result", result);
        
        // 生成JSON字符串
        char *json_str = cJSON_PrintUnformatted(root);
        if (!json_str) {
            free(base64_buffer);
            cJSON_Delete(root);
            return -1;
        }
        
        // 检查缓冲区大小
        if (strlen(json_str) > max_len) {
            // LOGD("JSON string length exceeds max_len\n");
            log_write(LOG_ERROR, TAG_NAME, "JSON string length exceeds max_len\n");
            free(base64_buffer);
            cJSON_free(json_str);
            cJSON_Delete(root);
            return -1;
        }
        
        // 复制JSON响应
        strcpy(response, json_str);
        
        free(base64_buffer);
        cJSON_free(json_str);
        cJSON_Delete(root);
        
        return strlen(response);
    }
    #else
    {
        // 二进制模式
        cJSON_AddNumberToObject(result, "length", photo->data_len);
        cJSON_AddItemToObject(root, "result", result);

        // 添加二进制数据标记
        const char *separator = "<<BINARY>>";
        
        // 生成JSON字符串
        char *json_str = cJSON_PrintUnformatted(root);
        if (!json_str) {
            cJSON_Delete(root);
            return -1;
        }

        // 组装完整响应
        int json_len = strlen(json_str);
        int separator_len = strlen(separator);
        
        // 检查缓冲区大小
        if (json_len + separator_len + photo->data_len > max_len) {
            cJSON_free(json_str);
            cJSON_Delete(root);
            return -1;
        }

        // 拼接数据
        memcpy(response, json_str, json_len);
        memcpy(response + json_len, separator, separator_len);
        memcpy(response + json_len + separator_len, photo->data, photo->data_len);
        // printf("data len: %d\n", photo->data_len);
        // printf("data: ");
        // for(int i = 0; i < photo->data_len; i++)
        // {
        //     printf("%x ", photo->data[i]);
        // }
        printf("\n");
        cJSON_free(json_str);
        cJSON_Delete(root);
        printf("response len: %d\n", json_len + separator_len + photo->data_len);
        return json_len + separator_len + photo->data_len;
    }
    #endif
}
static void delete_img(char *img_path)
{
    char cmd_remove_path[256] = {0};
    snprintf(cmd_remove_path, sizeof(cmd_remove_path), "rm -rf %s", img_path);
    LOGD("remove [%s]\n", cmd_remove_path);
    system(cmd_remove_path);
}
int handle_trans_door_photo_request(char *response, int response_max_len, const char *request_data, int request_len)
{
    PhotoRequest photo_req = {0};
    
    // 解析请求
    if (parse_photo_request(request_data, request_len, &photo_req) != 0) {
        photo_req.header.result = 1;
        log_write(LOG_ERROR, TAG_NAME, "Failed to parse photo request");
        return generate_photo_response(response, response_max_len, &photo_req);
    }

/*获取图片路径*/
    char image_path[128] = {0};
    snprintf(image_path, sizeof(image_path), "/tmp/data/%d/img-%d-%d.jpg",
             photo_req.open_count, photo_req.open_count, photo_req.angle);


    // 确保存储目录存在
    if (access(image_path, F_OK) != 0) {
        /*目录不存在*/
        log_write(LOG_ERROR, TAG_NAME, "file not exist");
        photo_req.header.result = 1;
        return generate_photo_response(response, response_max_len, &photo_req);
    }
    // log_write(LOG_INFO, TAG_NAME, "image path : %s", image_path);

    // 读取照片数据
    FILE *fp = fopen(image_path, "rb");
    if (!fp) {
        log_write(LOG_ERROR, TAG_NAME, "Failed to open photo file: %s", strerror(errno));
        photo_req.header.result = 4;
        return generate_photo_response(response, response_max_len, &photo_req);
    }

    // 获取文件大小
    // fseek(fp, 0, SEEK_END);
    // photo_req.size = ftell(fp);
    // printf("file size: %lu\n", photo_req.size);
    struct stat st;
    if (stat(image_path, &st) == 0) {
        photo_req.size = st.st_size;
        printf("File size: %llu\n", (unsigned long long)photo_req.size);
    } else {
        printf("stat failed: %s\n", strerror(errno));
    }



    // 检查偏移量
    if (photo_req.offset >= photo_req.size) {
        fclose(fp);
        photo_req.header.result = 2;
        photo_req.data_len = 0;
        LOGD("offset >= size\n");
        return generate_photo_response(response, response_max_len, &photo_req);
    }

    // 读取请求的数据块
    fseek(fp, photo_req.offset, SEEK_SET);
    photo_req.data_len = fread(photo_req.data, 1, 
                              photo_req.length > MAX_PHOTO_BUFFER_SIZE ? 
                              MAX_PHOTO_BUFFER_SIZE : photo_req.length, 
                              fp);
    if (photo_req.data_len < 0) {
        // LOGD("Error reading photo data: %s\n", strerror(errno));
        log_write(LOG_ERROR, TAG_NAME, "Error reading photo data: %s\n", strerror(errno));
        fclose(fp);
        photo_req.header.result = 6; // 使用一个合适的错误码
        return generate_photo_response(response, response_max_len, &photo_req);
    } else if (photo_req.data_len == 0 && ferror(fp)) {
        // LOGD("Error reading photo data: %s\n", strerror(errno));
        log_write(LOG_ERROR, TAG_NAME, "Error reading photo data: %s\n", strerror(errno));
        fclose(fp);
        photo_req.header.result = 6; // 使用一个合适的错误码
        return generate_photo_response(response, response_max_len, &photo_req);
    } else if (photo_req.data_len == 0 && feof(fp)) {
        // LOGD("End of file reached unexpectedly\n");
        log_write(LOG_ERROR, TAG_NAME, "End of file reached unexpectedly\n");
        fclose(fp);
        photo_req.header.result = 7; // 使用一个合适的错误码
        return generate_photo_response(response, response_max_len, &photo_req);
    }

    fclose(fp);


    int progress = (photo_req.offset + photo_req.data_len) * 100 / photo_req.size;
    LOGD("Transfer progress: %d%% (%d/%lu bytes)\n", 
         progress, photo_req.offset + photo_req.data_len, photo_req.size);

    // 检查是否是最后一帧
    if (photo_req.offset + photo_req.data_len >= photo_req.size) {
        printf("file size: %lu\n", photo_req.size);
        log_write(LOG_INFO, TAG_NAME, "Transfer completed successfully: %s", image_path);
        delete_img(image_path);  // 删除临时文件
        log_write(LOG_INFO, TAG_NAME, "Temporary file deleted: %s", image_path);
    }

    // 生成响应
    photo_req.header.result = 0;
    // LOGD("response : %s\n", response);
    return generate_photo_response(response, response_max_len, &photo_req);
}