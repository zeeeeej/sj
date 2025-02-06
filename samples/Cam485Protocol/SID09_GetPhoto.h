#ifndef SID09_GET_PHOTO___
#define SID09_GET_PHOTO___

#include "stdint.h"
#include "MsgDispatcher.h"

#define BUFFER_SIZE 4096

typedef struct {
    uint8_t header[2];        // 帧头，例如 0xAA 0x5A
    uint8_t slave_addr;
    uint8_t command;          // 主命令，固定为 0x09
    uint32_t payload_len;  
    uint8_t pic_id;
    uint32_t offset;          // 偏移量，从图片数据的第 offset 字节开始读取
    uint32_t read_len;        // 读取的字节数
    uint8_t result;        // 结果
} GetPictureRequest_t;

int SID09_GetPhoto(uint8_t *msg_buf, uint32_t msg_len);
void send_picture_data(uint8_t *msg_buf, uint32_t msg_len);
#endif