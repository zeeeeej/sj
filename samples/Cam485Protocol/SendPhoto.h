#ifndef _SEMDPHOTO_H_
#define _SEMDPHOTO_H_

#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include "sys/stat.h"
#include "unistd.h"
#include "circular_log.h"

typedef struct {
    uint8_t header[2];        // 帧头，例如 0xAA 0x5A
    uint8_t command;          // 主命令，固定为 0x09
    uint8_t sub_command;      // 子命令，例如 0x01 表示获取图片
    uint8_t pic_id;           // 图片 ID
    uint32_t offset;          // 偏移量，从图片数据的第 offset 字节开始读取
    uint32_t read_len;        // 读取的字节数
    uint16_t checksum;        // 校验和
} GetPictureRequest_t;


typedef struct {
    uint8_t header[2];        // 帧头，例如 0xAA 0x5A
    uint8_t command;          // 主命令，固定为 0x09
    uint8_t len;              // 数据长度（后续数据的总长度）
    uint8_t result;           // 返回值：0x00 表示成功，其他值表示失败
    uint8_t pic_data[];       // 图片数据（如果 result 为 0x00，则包含图片数据；失败时为空）
} GetPictureResponse_t;

#define TAKE_PHOTO_TMP_DIR "/tmp/take_photo"
#define TAKE_PHOTO_TMP_FILE TAKE_PHOTO_TMP_DIR "/photo.jpg"
#define TAKE_PHOTO_BUFFER_SIZE 8192
#define PHOTO_STORAGE_DIR "/tmp/camera"
#define PHOTO_FILE_PATH "/tmp/camera/current.jpg"
#define MAX_PHOTO_BUFFER_SIZE 8000
#define TRANSFER_MODE_BINARY 0
#define TRANSFER_MODE_BASE64 1
#define CURRENT_TRANSFER_MODE  TRANSFER_MODE_BINARY
#define TAG_NAME "take_photo"

int ParseGetPictureRequest(const uint8_t *msg_buf, uint16_t msg_len,GetPictureRequest_t *req);
int PackGetPictureResponse(uint8_t *msg_buf, uint16_t buf_size, uint8_t photo_len,const uint8_t result, const uint8_t *photo_data);
int HandlePhotoRequest(const uint8_t *msg_buf, uint16_t msg_len);


#endif // _SEMDPHOTO_H_