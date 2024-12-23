//
// Created by v on 24-10-28.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "wind_protocol_wrap_up.h"
#include "cJSON.h"
#include "cm_common.h"
#include "cm_base64.h"
#define TRANSFER_MODE_BASE64 1
#define TRANSFER_MODE_BINARY 2
#define CURRENT_TRANSFER_MODE TRANSFER_MODE_BASE64  // 可配置的传输模式
static uint32_t s_seq_num = 0;
WindProOtaRequest ota_req;
static uint32_t getSeq()
{
    if (s_seq_num >= INT32_MAX)
    {
        s_seq_num = 1;
    }
    return s_seq_num++;
}

int wind_protocol_wrap_up_parse_header(char *data, int len, WindProUpdateHeader *header)
{
    cJSON *root = cJSON_ParseWithLength(data, len);
    if (root)
    {
        cJSON *obj = cJSON_GetObjectItem(root, "timestamp");
        if (obj)
        {
            header->timestamp = (uint64_t)cJSON_GetNumberValue(obj);
        }
        obj = cJSON_GetObjectItem(root, "seqNum");
        if (obj)
        {
            header->seqNum = (uint32_t)cJSON_GetNumberValue(obj);
        }
        obj = cJSON_GetObjectItem(root, "cmdType");
        if (obj)
        {
            header->cmdType = (uint32_t)cJSON_GetNumberValue(obj);
        }
        cJSON_Delete(root);
        return 0;
    }
    else
    {
        cJSON_Delete(root);
        return -2;
    }
    return -1;
}

int wind_protocol_wrap_up_parse_response_version(char *data, int len, WindProUpdateVersion *version)
{
    cJSON *root = cJSON_ParseWithLength(data, len);
    if (root)
    {
        cJSON *obj = cJSON_GetObjectItem(root, "result");
        if (obj)
        {
            cJSON *obj_sub = cJSON_GetObjectItem(obj, "version");
            if (obj_sub)
            {
                strncpy(version->version, cJSON_GetStringValue(obj_sub), sizeof(version->version));
                cJSON_Delete(root);
                return 0;
            }
        }
    }
    cJSON_Delete(root);
    LOGD("Error [%s]\n", data);
    return -1;
}

int wind_protocol_wrap_up_gen_request_version(char *data, int len)
{
    cJSON *root = cJSON_CreateObject();
    uint32_t seq = getSeq();
    cJSON_AddNumberToObject(root, "timestamp", seq);
    cJSON_AddNumberToObject(root, "seqNum", seq);
    cJSON_AddNumberToObject(root, "cmdType", WIND_PRO_UP_CMD_VERSION);
    cJSON_AddStringToObject(root, "info", "");
    char *d = cJSON_PrintUnformatted(root);
    strncpy(data, d, strlen(d) > len ? len : strlen(d));
    cJSON_free(d);
    cJSON_Delete(root);
    return strlen(data);
}
int wind_serial_request_connect(char *data, int len, WindProUpdateHeader *header)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "timestamp", header->timestamp);
    cJSON_AddNumberToObject(root, "seqNum", header->seqNum);
    cJSON_AddNumberToObject(root, "cmdType", header->cmdType);
    cJSON_AddNumberToObject(root, "result", 1);
    char *d = cJSON_PrintUnformatted(root);
    strncpy(data, d, strlen(d) > len ? len : strlen(d));
    cJSON_free(d);
    cJSON_Delete(root);
    return strlen(data);
}

int wind_serial_reponse_reboot(char *data, int len, WindProUpdateHeader *header)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "timestamp", header->timestamp);
    cJSON_AddNumberToObject(root, "seqNum", header->seqNum);
    cJSON_AddNumberToObject(root, "cmdType", header->cmdType);
    cJSON_AddNumberToObject(root, "result", 1);
    char *d = cJSON_PrintUnformatted(root);
    strncpy(data, d, strlen(d) > len ? len : strlen(d));
    cJSON_free(d);
    cJSON_Delete(root);
    return strlen(data);
}

int wind_protocol_wrap_up_gen_response_version(char *data, int len, WindProUpdateHeader *header, char *version)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "timestamp", header->timestamp);
    cJSON_AddNumberToObject(root, "seqNum", header->seqNum);
    cJSON_AddNumberToObject(root, "cmdType", header->cmdType);
    cJSON *obj = cJSON_CreateObject();
    cJSON_AddStringToObject(obj, "version", version);
    cJSON_AddItemToObject(root, "result", obj);
    char *d = cJSON_PrintUnformatted(root);
    strncpy(data, d, strlen(d) > len ? len : strlen(d));
    cJSON_free(d);
    cJSON_Delete(root);
    return strlen(data);
}

int wind_protocol_wrap_up_gen_request_query_system(char *data, int len)
{
    cJSON *root = cJSON_CreateObject();
    uint32_t seq = getSeq();
    cJSON_AddNumberToObject(root, "timestamp", seq);
    cJSON_AddNumberToObject(root, "seqNum", seq);
    cJSON_AddNumberToObject(root, "cmdType", WIND_PRO_UP_CMD_QUERY_SYSTEM);
    cJSON_AddStringToObject(root, "info", "");
    char *d = cJSON_PrintUnformatted(root);
    strncpy(data, d, strlen(d) > len ? len : strlen(d));
    cJSON_free(d);
    cJSON_Delete(root);
    return strlen(data);
}

int wind_protocol_wrap_up_gen_response_query_system(char *data, int len, WindProUpdateSystemInfo *info)
{
    // cJSON *root = cJSON_CreateObject();
    // cJSON_AddNumberToObject(root, "timestamp", info->header.timestamp);
    // cJSON_AddNumberToObject(root, "seqNum", info->header.seqNum);
    // cJSON_AddNumberToObject(root, "cmdType", info->header.cmdType);
    // cJSON *obj = cJSON_CreateObject();
    // if (info->header.result == 0)
    // {
    //     cJSON_AddNumberToObject(obj, "cmd", info->cmd);
    //     cJSON_AddNumberToObject(obj, "open_count", info->open_count);
    //     cJSON_AddNumberToObject(obj, "angle", info->angle);
    // }
    // cJSON_AddNumberToObject(obj, "error", info->header.result);

    // cJSON_AddItemToObject(root, "result", obj);
    // char *d = cJSON_PrintUnformatted(root);
    // strncpy(data, d, strlen(d) > len ? len : strlen(d));
    // cJSON_free(d);
    // cJSON_Delete(root);
    // return strlen(data);

    cJSON *root = cJSON_CreateObject();
    
    // 添加基本字段
    cJSON_AddNumberToObject(root, "timestamp", info->header.timestamp);
    cJSON_AddNumberToObject(root, "seqNum", info->header.seqNum);
    cJSON_AddNumberToObject(root, "cmdType", info->header.cmdType);

    // 根据result判断是否有事件
    if (info->header.result == 0) {
        // 有事件时
        cJSON_AddNumberToObject(root, "eventType", info->cmd);

        // 创建data对象并添加详细信息
        cJSON *data_obj = cJSON_CreateObject();
        cJSON_AddNumberToObject(data_obj, "open_count", info->open_count);
        cJSON_AddNumberToObject(data_obj, "angle", info->angle);
        cJSON_AddNumberToObject(data_obj, "error", 0);  // 有事件时error为0
        cJSON_AddItemToObject(root, "data", data_obj);
    } else {
        // 无事件时
        cJSON_AddNumberToObject(root, "eventType", 0);  // 无事件时eventType为0

        // 创建空的data对象，只包含error
        cJSON *data_obj = cJSON_CreateObject();
        cJSON_AddNumberToObject(data_obj, "error", 1);  // 无事件时error为1
        cJSON_AddItemToObject(root, "data", data_obj);
    }

    // 序列化JSON并复制到输出buffer
    char *json_str = cJSON_PrintUnformatted(root);
    if (json_str) {
        size_t json_len = strlen(json_str);
        strncpy(data, json_str, json_len < len ? json_len : len);
        data[len - 1] = '\0';  // 确保字符串结束
        cJSON_free(json_str);
    }
    
    cJSON_Delete(root);
    return strlen(data);
}


/*camera自检属性*/
#include "self_check.h"
int wind_protocol_wrap_up_gen_response_self_check_result(char *data, int len,WindProUpdateHeader *header)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "timestamp", header->timestamp);
    cJSON_AddNumberToObject(root, "seqNum", header->seqNum);
    cJSON_AddNumberToObject(root, "cmdType", header->cmdType);
    cJSON_AddNumberToObject(root, "result", 1);

    SelfCheckProperty *CamcheckProperty = self_check_get_cam_check_property();
    SelfCheckProperty *GyrocheckProperty = self_check_get_gyro_check_property();
    int self_check_flag = self_check_get_status();
    /*摄像头*/

    /*返回自检结果*/
    cJSON_AddNumberToObject(root, "status", self_check_flag);
    cJSON_AddNumberToObject(root, "camera", CamcheckProperty->result);
    cJSON_AddNumberToObject(root, "Gyro", GyrocheckProperty->result);

    // cJSON_AddNumberToObject(root, "camera", 0);
    // cJSON_AddNumberToObject(root, "Gyro", 0);
    char *d = cJSON_PrintUnformatted(root);

    strncpy(data, d, strlen(d) > len ? len : strlen(d));
    cJSON_free(d);
    cJSON_Delete(root);
    return strlen(data);
}

//

int wind_protocol_wrap_up_parse_response_query_system(char *data, int len, WindProUpdateSystemInfo *info)
{
    cJSON *root = cJSON_ParseWithLength(data, len);
    if (root)
    {
        cJSON *obj = cJSON_GetObjectItem(root, "result");
        if (obj)
        {
            cJSON *obj_sub = cJSON_GetObjectItem(obj, "error");
            if (obj_sub)
            {
                info->header.result = cJSON_GetNumberValue(obj_sub);
            }
            obj_sub = cJSON_GetObjectItem(obj, "cmd");
            if (obj_sub)
            {
                info->cmd = cJSON_GetNumberValue(obj_sub);
            }
            obj_sub = cJSON_GetObjectItem(obj, "open_count");
            if (obj_sub)
            {
                info->open_count = cJSON_GetNumberValue(obj_sub);
            }
            obj_sub = cJSON_GetObjectItem(obj, "angle");
            if (obj_sub)
            {
                info->angle = cJSON_GetNumberValue(obj_sub);
            }
            cJSON_Delete(root);
            return 0;
        }
    }
    cJSON_Delete(root);
    LOGD("Error [%s]\n", data);
    return -1;
}

int wind_protocol_wrap_up_gen_request_query_config(char *data, int len)
{
    cJSON *root = cJSON_CreateObject();
    uint32_t seq = getSeq();
    cJSON_AddNumberToObject(root, "timestamp", seq);
    cJSON_AddNumberToObject(root, "seqNum", seq);
    cJSON_AddNumberToObject(root, "cmdType", WIND_PRO_UP_CMD_QUERY_CONFIG);
    cJSON_AddStringToObject(root, "info", "");
    char *d = cJSON_PrintUnformatted(root);
    strncpy(data, d, strlen(d) > len ? len : strlen(d));
    cJSON_free(d);
    cJSON_Delete(root);
    return strlen(data);
}

int wind_protocol_wrap_up_gen_response_query_config(char *data, int len, WindProUpdateConfig *config)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "timestamp", config->header.timestamp);
    cJSON_AddNumberToObject(root, "seqNum", config->header.seqNum);
    cJSON_AddNumberToObject(root, "cmdType", config->header.cmdType);
    cJSON *obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(obj, "door_open_angle", config->door_open_angle);
    cJSON_AddNumberToObject(obj, "capture_angle", config->capture_angle);
    cJSON_AddNumberToObject(obj, "door_open_timeout", config->door_open_timeout);

    cJSON_AddItemToObject(root, "result", obj);
    char *d = cJSON_PrintUnformatted(root);
    strncpy(data, d, strlen(d) > len ? len : strlen(d));
    cJSON_free(d);
    cJSON_Delete(root);
    return strlen(data);
}

int wind_protocol_wrap_up_parse_response_query_config(char *data, int len, WindProUpdateConfig *config)
{
    cJSON *root = cJSON_ParseWithLength(data, len);
    if (root)
    {
        cJSON *obj = cJSON_GetObjectItem(root, "result");
        if (obj)
        {
            cJSON *obj_sub = cJSON_GetObjectItem(obj, "door_open_angle");
            if (obj_sub)
            {
                config->door_open_angle = cJSON_GetNumberValue(obj_sub);
            }
            obj_sub = cJSON_GetObjectItem(obj, "capture_angle");
            if (obj_sub)
            {
                config->capture_angle = cJSON_GetNumberValue(obj_sub);
            }
            obj_sub = cJSON_GetObjectItem(obj, "door_open_timeout");
            if (obj_sub)
            {
                config->door_open_timeout = cJSON_GetNumberValue(obj_sub);
            }
            cJSON_Delete(root);
            return 0;
        }
    }
    cJSON_Delete(root);
    LOGD("Error [%s]\n", data);
    return -1;
}

int wind_protocol_wrap_up_gen_request_set_config(char *data, int len, WindProUpdateConfig *config)
{
    cJSON *root = cJSON_CreateObject();
    uint32_t seq = getSeq();
    cJSON_AddNumberToObject(root, "timestamp", seq);
    cJSON_AddNumberToObject(root, "seqNum", seq);
    cJSON_AddNumberToObject(root, "cmdType", WIND_PRO_UP_CMD_SET_CONFIG);

    cJSON *obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(obj, "door_open_angle", config->door_open_angle);
    cJSON_AddNumberToObject(obj, "capture_angle", config->capture_angle);
    cJSON_AddNumberToObject(obj, "door_open_timeout", config->door_open_timeout);

    cJSON_AddItemToObject(root, "info", obj);
    char *d = cJSON_PrintUnformatted(root);
    strncpy(data, d, strlen(d) > len ? len : strlen(d));
    cJSON_free(d);
    cJSON_Delete(root);
    return strlen(data);
}

int wind_protocol_wrap_up_gen_response_set_config(char *data, int len, WindProUpdateConfig *config, int result)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "timestamp", config->header.timestamp);
    cJSON_AddNumberToObject(root, "seqNum", config->header.seqNum);
    cJSON_AddNumberToObject(root, "cmdType", config->header.cmdType);
    cJSON *obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(obj, "error", result);

    cJSON_AddItemToObject(root, "result", obj);
    char *d = cJSON_PrintUnformatted(root);
    strncpy(data, d, strlen(d) > len ? len : strlen(d));
    cJSON_free(d);
    cJSON_Delete(root);
    return strlen(data);
}

int wind_protocol_wrap_up_parse_response_set_config(char *data, int len)
{
    cJSON *root = cJSON_ParseWithLength(data, len);
    if (root)
    {
        cJSON *obj = cJSON_GetObjectItem(root, "result");
        if (obj)
        {
            cJSON *obj_sub = cJSON_GetObjectItem(obj, "error");
            if (obj_sub)
            {
                int err = cJSON_GetNumberValue(obj_sub);
                return err == 0 ? 0 : -3;
            }
            cJSON_Delete(root);
            return -2;
        }
    }
    cJSON_Delete(root);
    LOGD("Error [%s]\n", data);
    return -1;
}

int wind_protocol_wrap_up_parse_request_set_config(char *data, int len, WindProUpdateConfig *config)
{
    cJSON *root = cJSON_ParseWithLength(data, len);
    if (root)
    {
        cJSON *obj = cJSON_GetObjectItem(root, "info");
        if (obj)
        {
            cJSON *obj_sub = cJSON_GetObjectItem(obj, "door_open_angle");
            if (obj_sub)
            {
                config->door_open_angle = cJSON_GetNumberValue(obj_sub);
            }
            obj_sub = cJSON_GetObjectItem(obj, "capture_angle");
            if (obj_sub)
            {
                config->capture_angle = cJSON_GetNumberValue(obj_sub);
            }
            obj_sub = cJSON_GetObjectItem(obj, "door_open_timeout");
            if (obj_sub)
            {
                config->door_open_timeout = cJSON_GetNumberValue(obj_sub);
            }
            cJSON_Delete(root);
            return 0;
        }
    }
    cJSON_Delete(root);
    LOGD("Error [%s]\n", data);
    return -1;
}

int wind_protocol_wrap_up_gen_request_usb_stream(char *data, int len, int open)
{
    cJSON *root = cJSON_CreateObject();
    uint32_t seq = getSeq();
    cJSON_AddNumberToObject(root, "timestamp", seq);
    cJSON_AddNumberToObject(root, "seqNum", seq);
    cJSON_AddNumberToObject(root, "cmdType", open == 1 ? WIND_PRO_UP_CMD_OPEN_USB_STREAM : WIND_PRO_UP_CMD_CLOSE_USB_STREAM);

    cJSON_AddStringToObject(root, "info", "");
    char *d = cJSON_PrintUnformatted(root);
    strncpy(data, d, strlen(d) > len ? len : strlen(d));
    cJSON_free(d);
    cJSON_Delete(root);
    return strlen(data);
}
int wind_protocol_wrap_up_gen_response_ota(char *data, int len, WindProUpdateHeader *header)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "timestamp", header->timestamp);
    cJSON_AddNumberToObject(root, "seqNum", header->seqNum);
    cJSON_AddNumberToObject(root, "cmdType", header->cmdType);
    cJSON_AddNumberToObject(root, "result", 1);
    char *d = cJSON_PrintUnformatted(root);
    strncpy(data, d, strlen(d) > len ? len : strlen(d));
    cJSON_free(d);
    cJSON_Delete(root);
    return strlen(data);
}
int wind_protocol_wrap_up_gen_response_usb_stream(char *data, int len, int result, WindProUpdateHeader *header)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "timestamp", header->timestamp);
    cJSON_AddNumberToObject(root, "seqNum", header->seqNum);
    cJSON_AddNumberToObject(root, "cmdType", header->cmdType);
    cJSON *obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(obj, "error", result);

    cJSON_AddItemToObject(root, "result", obj);
    char *d = cJSON_PrintUnformatted(root);
    strncpy(data, d, strlen(d) > len ? len : strlen(d));
    cJSON_free(d);
    cJSON_Delete(root);
    return strlen(data);
}

int wind_protocol_wrap_up_parse_response_usb_stream(char *data, int len)
{
    cJSON *root = cJSON_ParseWithLength(data, len);
    if (root)
    {
        cJSON *obj = cJSON_GetObjectItem(root, "result");
        if (obj)
        {
            cJSON *obj_sub = cJSON_GetObjectItem(obj, "error");
            if (obj_sub)
            {
                int err = cJSON_GetNumberValue(obj_sub);
                return err == 0 ? 0 : -3;
            }
            cJSON_Delete(root);
            return -2;
        }
    }
    cJSON_Delete(root);
    LOGD("Error [%s]\n", data);
    return -1;
}

int wind_protocol_wrap_up_gen_request_photo(char *data, int len, WindProUpdateQueryPhoto *photo)
{
    cJSON *root = cJSON_CreateObject();
    uint32_t seq = getSeq();
    cJSON_AddNumberToObject(root, "timestamp", seq);
    cJSON_AddNumberToObject(root, "seqNum", seq);
    cJSON_AddNumberToObject(root, "cmdType", WIND_PRO_UP_CMD_QUERY_PHOTO);

    cJSON *obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(obj, "open_count", photo->open_count);
    cJSON_AddNumberToObject(obj, "angle", photo->angle);
    cJSON_AddNumberToObject(obj, "offset", photo->offset);
    cJSON_AddNumberToObject(obj, "length", photo->length);

    cJSON_AddItemToObject(root, "info", obj);
    char *d = cJSON_PrintUnformatted(root);
    strncpy(data, d, strlen(d) > len ? len : strlen(d));
    cJSON_free(d);
    cJSON_Delete(root);
    return strlen(data);
}

int wind_protocol_wrap_up_gen_response_photo(char *data, int len, WindProUpdateQueryPhoto *photo, WindProUpdateHeader *header)
{
    LOGD("------ [%d]---------", photo->data_len);
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "timestamp", photo->header.timestamp);
    cJSON_AddNumberToObject(root, "seqNum", photo->header.seqNum);
    cJSON_AddNumberToObject(root, "cmdType", photo->header.cmdType);
    cJSON *obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(obj, "error", photo->header.result);
    cJSON_AddNumberToObject(obj, "size", photo->size);
    // save data
    char outdata[8192] = {0};
    size_t d_str_len = sizeof(outdata);
    cm_base64_encode((unsigned char *)outdata, &d_str_len, photo->data, photo->data_len);
    cJSON_AddStringToObject(obj, "data", outdata);
    cJSON_AddNumberToObject(obj, "length", d_str_len);
    LOGD("encode data [%02x %02x %02x %02x]\n", outdata[0], outdata[1], outdata[2], outdata[3]);
    LOGD("read out data len [%d] encode len [%u]\n", photo->data_len, d_str_len);

    /*====add system event report====*/
    WindSystemEvent *event = wind_system_event_pop();
    WindProUpdateSystemInfo info;
    if (event)
    {

        info.header = *header;
        info.header.result = 0;
        info.cmd = event->info.cmd;
        info.angle = event->info.angle;
        info.open_count = event->info.open_count;
        free(event);
    }
    else
    {
        info.header = *header;
        info.header.result = 99;
        // return wind_protocol_wrap_up_gen_response_query_system((char *)res, len, &info);
    }
    cJSON *system_event = cJSON_CreateObject();
    if (info.header.result == 0)
    {
        cJSON_AddNumberToObject(system_event, "cmd", info.cmd);
        cJSON_AddNumberToObject(system_event, "open_count", info.open_count);
        cJSON_AddNumberToObject(system_event, "angle", info.angle);
    }
    cJSON_AddNumberToObject(system_event, "error", info.header.result);
    cJSON_AddItemToObject(root, "system_event", system_event);
    /*=====add system event report=====*/

    cJSON_AddItemToObject(root, "result", obj);
    char *d = cJSON_PrintUnformatted(root);
    strncpy(data, d, strlen(d) > len ? len : strlen(d));
    cJSON_free(d);
    cJSON_Delete(root);
    return strlen(data);
}

int wind_protocol_wrap_up_parse_response_photo(char *data, int len, WindProUpdateQueryPhoto *photo)
{
    cJSON *root = cJSON_ParseWithLength(data, len);
    if (root)
    {
        cJSON *obj = cJSON_GetObjectItem(root, "result");
        if (obj)
        {
            cJSON *obj_sub = cJSON_GetObjectItem(obj, "error");
            if (obj_sub)
            {
                int err = cJSON_GetNumberValue(obj_sub);
                photo->header.result = err;
            }
            obj_sub = cJSON_GetObjectItem(obj, "size");
            if (obj_sub)
            {
                photo->size = cJSON_GetNumberValue(obj_sub);
            }
            obj_sub = cJSON_GetObjectItem(obj, "length");
            if (obj_sub)
            {
                photo->data_len = cJSON_GetNumberValue(obj_sub);
            }
            obj_sub = cJSON_GetObjectItem(obj, "data");
            if (obj_sub)
            {
                char *str = cJSON_GetStringValue(obj_sub);
                size_t d_str_len = strlen(str);
                cm_base64_decode((unsigned char *)str, &d_str_len, photo->data, sizeof(photo->data));
                photo->data_len = d_str_len;
            }
            cJSON_Delete(root);
            return -2;
        }
    }
    cJSON_Delete(root);
    LOGD("Error [%s]\n", data);
    return -1;
}

int wind_protocol_wrap_up_parse_request_photo(char *data, int len, WindProUpdateQueryPhoto *photo)
{
    cJSON *root = cJSON_ParseWithLength(data, len);
    if (root)
    {
        cJSON *obj = cJSON_GetObjectItem(root, "info");
        if (obj)
        {
            cJSON *obj_sub = cJSON_GetObjectItem(obj, "open_count");
            if (obj_sub)
            {
                photo->open_count = cJSON_GetNumberValue(obj_sub);
            }
            obj_sub = cJSON_GetObjectItem(obj, "length");
            if (obj_sub)
            {
                photo->length = cJSON_GetNumberValue(obj_sub);
            }
            obj_sub = cJSON_GetObjectItem(obj, "angle");
            if (obj_sub)
            {
                photo->angle = cJSON_GetNumberValue(obj_sub);
            }
            obj_sub = cJSON_GetObjectItem(obj, "offset");
            if (obj_sub)
            {
                photo->offset = cJSON_GetNumberValue(obj_sub);
            }
            cJSON_Delete(root);
            return -2;
        }
    }
    cJSON_Delete(root);
    LOGD("Error [%s]\n", data);
    return -1;
}

/**
 * @brief 解析OTA升级请求
 * @param data 接收到的JSON数据
 * @param len 数据长度
 * @param ota_req 解析后的OTA请求信息
 * @return 0:成功 -1:失败
 */
int wind_protocol_wrap_up_parse_request_ota(char *data, int len, WindProOtaRequest *ota_req)
{
    if (!data || !ota_req || len <= 0) {
        LOGD("Invalid parameters");
        return -1;
    }

    // 初始化结构体
    memset(ota_req, 0, sizeof(WindProOtaRequest));

    cJSON *root = cJSON_ParseWithLength(data, len);
    if (!root) {
        LOGD("JSON parse failed");
        return -1;
    }

    // 解析基本字段
    cJSON *timestamp = cJSON_GetObjectItem(root, "timestamp");
    cJSON *seqNum = cJSON_GetObjectItem(root, "seqNum");
    cJSON *cmdType = cJSON_GetObjectItem(root, "cmdType");

    if (!timestamp || !seqNum || !cmdType ||
        !cJSON_IsNumber(timestamp) ||
        !cJSON_IsNumber(seqNum) ||
        !cJSON_IsNumber(cmdType)) {
        LOGD("Missing or invalid basic fields");
        cJSON_Delete(root);
        return -1;
    }

    // 存储基本字段
    ota_req->timestamp = timestamp->valueint;
    ota_req->seq_num = seqNum->valueint;
    ota_req->cmd_type = cmdType->valueint;

    // 解析OTA相关字段
    cJSON *version = cJSON_GetObjectItem(root, "version");
    cJSON *file_size = cJSON_GetObjectItem(root, "file_size");
    cJSON *file_md5 = cJSON_GetObjectItem(root, "file_md5");

    if (!version || !file_size || !file_md5 ||
        !cJSON_IsString(version) ||
        !cJSON_IsNumber(file_size) ||
        !cJSON_IsString(file_md5)) {
        LOGD("Missing or invalid OTA fields");
        cJSON_Delete(root);
        return -1;
    }

    // 存储OTA字段
    strncpy(ota_req->version, version->valuestring, sizeof(ota_req->version) - 1);
    ota_req->file_size = file_size->valueint;
    strncpy(ota_req->file_md5, file_md5->valuestring, sizeof(ota_req->file_md5) - 1);

    // 检查可选字段
    cJSON *file_crc = cJSON_GetObjectItem(root, "file_crc");
    if (file_crc && cJSON_IsNumber(file_crc)) {
        ota_req->file_crc = file_crc->valueint;
        ota_req->has_crc = 1;
    }

    cJSON *file_type = cJSON_GetObjectItem(root, "file_type");
    if (file_type && cJSON_IsNumber(file_type)) {
        ota_req->file_type = file_type->valueint;
        ota_req->has_file_type = 1;
    }

    // 打印解析结果
    LOGD("Parsed OTA request:");
    LOGD("Version: %s", ota_req->version);
    LOGD("File size: %u", ota_req->file_size);
    LOGD("MD5: %s", ota_req->file_md5);
    if (ota_req->has_crc) {
        LOGD("CRC: %u", ota_req->file_crc);
    }
    if (ota_req->has_file_type) {
        LOGD("File type: %d", ota_req->file_type);
    }

    cJSON_Delete(root);
    return 0;
}

int wind_protocol_wrap_up_parse_ota_data_trans_packet(char *data, int len, WindProOtaInfo *ota_packet_info)
{
    if (!data || !ota_packet_info || len <= 0) {
        LOGD("Invalid parameters");
        return -1;
    }

    // 初始化结构体
    memset(ota_packet_info, 0, sizeof(WindProOtaInfo));
    ota_packet_info->data_p = ota_packet_info->data_buffer;  // 指向内部缓冲区

    cJSON *root = cJSON_ParseWithLength(data, len);
    if (!root) {
        LOGD("JSON parse failed");
        return -1;
    }

    // 解析基本字段
    cJSON *cmdType = cJSON_GetObjectItem(root, "cmdType");
    if (!cmdType || !cJSON_IsNumber(cmdType) ||
        cmdType->valueint != WIND_PRO_UP_CMD_OTA_DATA_TRANS_PACKET) {
        LOGD("Invalid command type");
        cJSON_Delete(root);
        return -1;
    }

    // 获取info对象
    cJSON *info = cJSON_GetObjectItem(root, "info");
    if (!info || !cJSON_IsObject(info)) {
        LOGD("Info object not found");
        cJSON_Delete(root);
        return -1;
    }

    // 解析OTA数据信息
    cJSON *offset = cJSON_GetObjectItem(info, "offset");
    cJSON *length = cJSON_GetObjectItem(info, "length");
    cJSON *totalSize = cJSON_GetObjectItem(info, "totalSize");
    cJSON *data_str = cJSON_GetObjectItem(info, "data");

    if (!offset || !cJSON_IsNumber(offset) ||
        !length || !cJSON_IsNumber(length) ||
        !totalSize || !cJSON_IsNumber(totalSize) ||
        !data_str || !cJSON_IsString(data_str)) {
        LOGD("Missing or invalid fields in info object");
        cJSON_Delete(root);
        return -1;
    }

    // 填充结构体基本信息
    ota_packet_info->offset = offset->valueint;
    ota_packet_info->length = length->valueint;
    ota_packet_info->total_size = totalSize->valueint;

    // 验证长度
    if (ota_packet_info->length > OTA_BUFFER_SIZE) {
        LOGD("Data length too large: %u, max allowed: %u", 
             ota_packet_info->length, OTA_BUFFER_SIZE);
        cJSON_Delete(root);
        return -1;
    }

    // 解码base64数据
    if (data_str->valuestring && strlen(data_str->valuestring) > 0) {
        size_t decoded_len = OTA_BUFFER_SIZE;  // 使用缓冲区最大大小
        
        if (cm_base64_decode(ota_packet_info->data_buffer, &decoded_len,
                           (const unsigned char *)data_str->valuestring,
                           strlen(data_str->valuestring)) != 0) {
            LOGD("Base64 decode failed");
            cJSON_Delete(root);
            return -1;
        }

        if (decoded_len != ota_packet_info->length) {
            LOGD("Decoded data length mismatch: expected %u, got %u",
                 ota_packet_info->length, decoded_len);
            cJSON_Delete(root);
            return -1;
        }
    } else {
        LOGD("Empty data field");
        cJSON_Delete(root);
        return -1;
    }

    cJSON_Delete(root);
    return 0;
}

int wind_protocol_wrap_up_gen_response_ota_trans_packet(char *data, int len, WindProUpdateHeader *header)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "timestamp", header->timestamp);
    cJSON_AddNumberToObject(root, "seqNum", header->seqNum);
    cJSON_AddNumberToObject(root, "cmdType", header->cmdType);
    cJSON_AddNumberToObject(root, "result", 1);
    // 在 info 对象中添加消息
    cJSON *info = cJSON_CreateObject();
    if (!info)
    {
        LOGD( "Failed to create info object");
        cJSON_Delete(root);
        return -1;
    }
    cJSON_AddStringToObject(info, "msg", "ok");

    // 将 info 对象添加到根对象
    cJSON_AddItemToObject(root, "info", info);
    char *d = cJSON_PrintUnformatted(root);
    strncpy(data, d, strlen(d) > len ? len : strlen(d));
    cJSON_free(d);
    cJSON_Delete(root);
    return strlen(data);
}

int wind_protocol_wrap_up_gen_response_ota_end_result(char *data, int len, WindProUpdateHeader *header,int result)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "timestamp", header->timestamp);
    cJSON_AddNumberToObject(root, "seqNum", header->seqNum);
    cJSON_AddNumberToObject(root, "cmdType", header->cmdType);
    cJSON_AddNumberToObject(root, "result", result);
    // 在 info 对象中添加消息
    cJSON *info = cJSON_CreateObject();
    if (!info)
    {
        LOGD( "Failed to create info object");
        cJSON_Delete(root);
        return -1;
    }
    cJSON_AddStringToObject(info, "msg", "ok");

    // 将 info 对象添加到根对象
    cJSON_AddItemToObject(root, "info", info);
    char *d = cJSON_PrintUnformatted(root);
    strncpy(data, d, strlen(d) > len ? len : strlen(d));
    cJSON_free(d);
    cJSON_Delete(root);
    return strlen(data);
}


int wind_protocol_wrap_up_take_photo_response(char *data, int len, WindProUpdateQueryPhoto *photo, WindProUpdateHeader *header)
{
    LOGD("------ [%d]---------", photo->data_len);
    cJSON *root = cJSON_CreateObject();
    if (!root) {
        LOGD("Failed to create JSON object");
        return -1;
    }

    // 添加基本信息
    cJSON_AddNumberToObject(root, "timestamp", photo->header.timestamp);
    cJSON_AddNumberToObject(root, "seqNum", photo->header.seqNum);
    cJSON_AddNumberToObject(root, "cmdType", photo->header.cmdType);
    
    cJSON *obj = cJSON_CreateObject();
    if (!obj) {
        cJSON_Delete(root);
        LOGD("Failed to create result object");
        return -1;
    }

    cJSON_AddNumberToObject(obj, "error", photo->header.result);
    cJSON_AddNumberToObject(obj, "size", photo->size);
    cJSON_AddNumberToObject(obj, "mode", CURRENT_TRANSFER_MODE);

    #if CURRENT_TRANSFER_MODE == TRANSFER_MODE_BASE64
        // Base64模式
        char outdata[8192] = {0};
        size_t d_str_len = sizeof(outdata);
        if (cm_base64_encode((unsigned char *)outdata, &d_str_len, photo->data, photo->data_len) != 0) {
            LOGD("Base64 encoding failed");
            cJSON_Delete(root);
            return -1;
        }
        cJSON_AddStringToObject(obj, "data", outdata);
        cJSON_AddNumberToObject(obj, "length", d_str_len);
        
        LOGD("Base64 mode - raw len: %d, encoded len: %u", photo->data_len, d_str_len);
        
        // 添加结果对象
        cJSON_AddItemToObject(root, "result", obj);
        
        // 生成JSON字符串
        char *json_str = cJSON_PrintUnformatted(root);
        if (!json_str) {
            cJSON_Delete(root);
            return -1;
        }
        
        size_t json_len = strlen(json_str);
        if (json_len > len) {
            LOGD("Buffer too small for JSON string");
            cJSON_free(json_str);
            cJSON_Delete(root);
            return -1;
        }
        
        memcpy(data, json_str, json_len);
        cJSON_free(json_str);
        cJSON_Delete(root);
        return json_len;
        
    #else
        // 二进制模式
        cJSON_AddNumberToObject(obj, "length", photo->data_len);
        cJSON_AddItemToObject(root, "result", obj);
        
        // 生成JSON头部
        char *json_str = cJSON_PrintUnformatted(root);
        if (!json_str) {
            cJSON_Delete(root);
            return -1;
        }
        
        // 计算总长度
        size_t json_len = strlen(json_str);
        const char *separator = "<<BINARY>>";
        size_t separator_len = strlen(separator);
        size_t total_len = json_len + separator_len + photo->data_len;
        
        if (total_len > len) {
            LOGD("Buffer too small for binary data");
            cJSON_free(json_str);
            cJSON_Delete(root);
            return -1;
        }
        
        // 组装数据包
        // 1. 复制JSON头部
        memcpy(data, json_str, json_len);
        // 2. 添加分隔符
        memcpy(data + json_len, separator, separator_len);
        // 3. 复制二进制数据
        memcpy(data + json_len + separator_len, photo->data, photo->data_len);
        
        LOGD("Binary mode - JSON len: %zu, data len: %d, total: %zu", 
             json_len, photo->data_len, total_len);
        
        cJSON_free(json_str);
        cJSON_Delete(root);
        return total_len;
    #endif
}