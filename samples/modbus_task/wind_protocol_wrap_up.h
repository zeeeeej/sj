//
// Created by v on 24-10-28.
//

#ifndef JK_START_WIND_PROTOCOL_WRAP_UPDATE_H
#define JK_START_WIND_PROTOCOL_WRAP_UPDATE_H

#include <stdint.h>
#include "cm_list.h"
enum
{
    WIND_PRO_UP_CMD_NONE = 0x0,
    WIND_PRO_UP_CMD_VERSION = 0x91,
    WIND_PRO_UP_CMD_QUERY_SYSTEM = 0x92,
    WIND_PRO_UP_CMD_QUERY_PHOTO = 0x93,
    WIND_PRO_UP_CMD_QUERY_CONFIG = 0x94,
    WIND_PRO_UP_CMD_SET_CONFIG = 0x95,
    WIND_PRO_UP_CMD_GET_SELF_CHECK_RESULT = 0x96,
    WIND_PRO_UP_CMD_REBOOT = 0x97,
    WIND_PRO_UP_CMD_REQUEST_CONNECT = 0x98,

    /*OTA升级相关指令*/
    WIND_PRO_UP_CMD_OTA = 0x99,                     /*OTA升级请求包*/
    WIND_PRO_UP_CMD_OTA_DATA_TRANS_PACKET = 0x9A,   /*OTA数据传输包*/
    WIND_PRO_UP_CMD_OTA_DATA_END = 0x9B,            /*OTA结果询问包*/

    /*拍照功能*/
    WIND_PRO_UP_CMD_TAKE_PHOTO = 0x9C,

    
    WIND_PRO_UP_CMD_OPEN_USB_STREAM = 0x9E,
    WIND_PRO_UP_CMD_CLOSE_USB_STREAM = 0x9F,
    WIND_PRO_UP_CMD_FILE_TRANS = 0xA0,
};

typedef struct
{
    uint64_t timestamp;
    uint32_t seqNum;
    uint32_t cmdType;
    int result;
} WindProUpdateHeader;

typedef struct
{
    WindProUpdateHeader header;
    char version[16];
} WindProUpdateVersion;

typedef struct
{
    WindProUpdateHeader header;
    int cmd; // 0 - door open, 1 - door close
    unsigned int open_count;
    int angle; // 当次拍照角度
} WindProUpdateSystemInfo;

typedef struct
{
    WindProUpdateHeader header;
    int door_open_angle;
    int capture_angle;
    uint32_t door_open_timeout;
} WindProUpdateConfig;


typedef struct
{
    WindProUpdateHeader header;
    uint32_t open_count;
    int angle; 
    uint64_t offset; // 读取照片偏移
    uint64_t length; // 读取照片长度

    // result
    uint64_t size;
    uint8_t  data[8192]; // 照片数据
    int data_len;
} WindProUpdateQueryPhoto;
typedef struct
{
    struct list_head test_list_item;
    WindProUpdateSystemInfo info;
} WindSystemEvent;


/**
 * @brief OTA数据结构体
 */
#define OTA_BUFFER_SIZE     5000
typedef struct
{
    uint32_t offset;                    // 数据偏移
    uint32_t length;                    // 数据长度
    uint32_t total_size;               // 总大小
    uint8_t data_buffer[OTA_BUFFER_SIZE];  // 固定大小的数据缓冲区
    uint8_t *data_p;                   // 指向data_buffer的指针
} WindProOtaInfo;

/**
 * @brief OTA请求信息结构体
 */
typedef struct {
    uint32_t timestamp;          // 时间戳
    uint32_t seq_num;           // 序列号
    uint32_t cmd_type;          // 命令类型
    char version[32];           // 版本号
    uint32_t file_size;         // 文件大小
    char file_md5[33];         // MD5值 (32字符 + '\0')
    uint32_t file_crc;         // CRC值 (可选)
    uint32_t file_type;        // 文件类型 (可选)
    uint8_t has_crc;           // 是否包含CRC
    uint8_t has_file_type;     // 是否包含文件类型
} WindProOtaRequest;

// parse and return header
WindSystemEvent *wind_system_event_pop();
int wind_serial_query_photo(unsigned char *res, int len, char *data, int data_len, WindProUpdateHeader *header);
int wind_protocol_wrap_up_parse_header(char *data, int len, WindProUpdateHeader *header);

int wind_protocol_wrap_up_parse_response_version(char *data, int len, WindProUpdateVersion *version);

int wind_protocol_wrap_up_gen_request_version(char *data, int len);
int wind_protocol_wrap_up_gen_response_version(char *data, int len, WindProUpdateHeader *header, char *version);

int wind_protocol_wrap_up_gen_request_query_system(char *data, int len);
int wind_protocol_wrap_up_gen_response_query_system(char *data, int len, WindProUpdateSystemInfo *info);
int wind_protocol_wrap_up_parse_response_query_system(char *data, int len, WindProUpdateSystemInfo *info);

int wind_protocol_wrap_up_gen_request_query_config(char *data, int len);
int wind_protocol_wrap_up_gen_response_query_config(char *data, int len, WindProUpdateConfig *config);
int wind_protocol_wrap_up_parse_response_query_config(char *data, int len, WindProUpdateConfig *config);

int wind_protocol_wrap_up_gen_request_set_config(char *data, int len, WindProUpdateConfig *config);
int wind_protocol_wrap_up_gen_response_set_config(char *data, int len, WindProUpdateConfig *config, int result);
int wind_protocol_wrap_up_parse_response_set_config(char *data, int len);
int wind_protocol_wrap_up_parse_request_set_config(char *data, int len, WindProUpdateConfig *config);

int wind_protocol_wrap_up_gen_request_usb_stream(char *data, int len, int open);
int wind_protocol_wrap_up_gen_response_usb_stream(char *data, int len, int result, WindProUpdateHeader *header);
int wind_protocol_wrap_up_parse_response_usb_stream(char *data, int len);

int wind_protocol_wrap_up_gen_request_photo(char *data, int len, WindProUpdateQueryPhoto *photo);
int wind_protocol_wrap_up_gen_response_photo(char *data, int len, WindProUpdateQueryPhoto *photo,WindProUpdateHeader *header);
int wind_protocol_wrap_up_parse_response_photo(char *data, int len, WindProUpdateQueryPhoto *photo);
int wind_protocol_wrap_up_parse_request_photo(char *data, int len, WindProUpdateQueryPhoto *photo);
int wind_protocol_wrap_up_gen_response_self_check_result(char *data, int len,WindProUpdateHeader *header);
int wind_serial_request_connect( char *res, int len, WindProUpdateHeader *header);
int wind_serial_reponse_reboot( char *data, int len, WindProUpdateHeader *header);

int wind_protocol_wrap_up_gen_response_ota(char *data, int len, WindProUpdateHeader *header);
int wind_protocol_wrap_up_parse_request_ota(char *data, int len, WindProOtaRequest *ota_req);
int wind_protocol_wrap_up_parse_ota_data_trans_packet(char *data, int len,WindProOtaInfo *ota_packet_info);
int wind_protocol_wrap_up_gen_response_ota_trans_packet(char *data, int len, WindProUpdateHeader *header);
int wind_protocol_wrap_up_gen_response_ota_end_result(char *data, int len, WindProUpdateHeader *header,int result);


int wind_protocol_wrap_up_take_photo_response(char *data, int len, WindProUpdateQueryPhoto *photo, WindProUpdateHeader *header);
#endif // JK_START_WIND_PROTOCOL_WRAP_UPDATE_H
