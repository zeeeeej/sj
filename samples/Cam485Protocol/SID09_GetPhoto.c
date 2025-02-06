#include "stdio.h"
#include "malloc.h"
#include <string.h>  // 包含 strerror 声明
#include <errno.h>   // 包含 errno 定义
#include <limits.h>  // 包含 PATH_MAX 定义
#include <stdbool.h> // 包含 bool 定义

#include "cm_common.h"
#include "SID09_GetPhoto.h"
#include "Cam485ProtocolCommon.h"
#include "MsgDispatcher.h"
#include "ImageInfoList.h"

#define RESPSIZE                (5000)

static char resp_buf[RESPSIZE];

static int ParseGetPictureRequest(const uint8_t *msg_buf, uint32_t msg_len,GetPictureRequest_t *req)
{
    if (msg_buf == NULL || req == NULL || msg_len < 19) 
    {
        LOGD("error:msg_len = %d",msg_len);
        return -1;
    }
    if (msg_buf[0] != 0xAA || msg_buf[1] != 0x5A || msg_buf[2] != 0x01 || msg_buf[3] != 0x09) 
    {
        return -2;
    }
    req->header[0] = msg_buf[0];
    req->header[1] = msg_buf[1];
    req->slave_addr = msg_buf[2];
    req->command = msg_buf[3];
    req->payload_len = (msg_buf[4] << 24) | (msg_buf[5] << 16) | (msg_buf[6] << 8) | msg_buf[7];
    // req->payload_len = (msg_buf[7] << 24) | (msg_buf[6] << 16) | (msg_buf[5] << 8) | msg_buf[4];
    req->pic_id = msg_buf[8];
    req->offset = (msg_buf[12] << 24) | (msg_buf[11] << 16) | (msg_buf[10] << 8) | msg_buf[9];
    req->read_len = (msg_buf[16] << 24) | (msg_buf[15] << 16) | (msg_buf[14] << 8) | msg_buf[13];

    /*单次传输获取的数据不能超过5000，5000是resp的容量，这里取4950，剩余的给包头和校验等*/
    // if (req->read_len > RESPSIZE-50)
    // {
    //     LOGD("error:read_len is too long! read_len = %d",req->read_len);
    //     return -3;
    // }

    // req->offset = (msg_buf[9] << 24) | (msg_buf[10] << 16) | (msg_buf[11] << 8) | msg_buf[12];
    // req->read_len = (msg_buf[13] << 24) | (msg_buf[14] << 16) | (msg_buf[15] << 8) | msg_buf[16];
    return 0;
}

/**
 * 打印 GetPictureRequest_t 结构体的内容
 * @param request 指向 GetPictureRequest_t 结构体的指针
 */
void print_get_picture_request(const GetPictureRequest_t *request) {
    if (request == NULL) {
        printf("Invalid request pointer.\n");
        return;
    }

    printf("GetPictureRequest_t contents:\n");
    printf("Picture ID: %u\n", request->pic_id);
    printf("Offset: %u\n", request->offset);
    printf("Read Length: %u\n", request->read_len);
    printf("Result: %d\n", request->result);
}


int PackGetPictureResponse(uint8_t *msg_buf, uint32_t data_payload_len, uint8_t* pic_data, uint32_t pic_data_len , uint8_t result)
{
    LOGD("pic data len : %d\n",pic_data_len);
    LOGD("data_payload_len : %d\n",data_payload_len);
    int index = 0;
    msg_buf[index++] = 0xAA;
    msg_buf[index++] = 0x5A;
    msg_buf[index++] = SLAVE_ADDR;
    msg_buf[index++] = 0x09;

    /*填充长度字段*/
    msg_buf[index++] = (uint8_t)(data_payload_len & 0xFF);         // Low byte
    msg_buf[index++] = (uint8_t)((data_payload_len >> 8) & 0xFF);  // 2nd byte
    msg_buf[index++] = (uint8_t)((data_payload_len >> 16) & 0xFF); // 3rd byte
    msg_buf[index++] = (uint8_t)((data_payload_len >> 24) & 0xFF); // High byte

    if(pic_data==NULL)
    {
        msg_buf[index++] = result;
        send_msg_image(resp_buf,data_payload_len+8);
    }
    else{
        msg_buf[index++] = result;
        memcpy(msg_buf+index ,pic_data,pic_data_len);
        send_msg_image(resp_buf,data_payload_len+8);
    }
}


int SID09_GetPhoto(uint8_t *msg_buf, uint32_t msg_len)
{
    GetPictureRequest_t req;
    int negative_payload_len = 1;
    int res = ParseGetPictureRequest(msg_buf, msg_len, &req);
    if(0 != res)
    {
        LOGD("Failed to parse photo request,ret[%d]\n",res);
        return PackGetPictureResponse(resp_buf,negative_payload_len,NULL,0,1);
    }
    if (req.command != 0x09) 
    {
        LOGD("Invalid command,ret[-2]\n");
        return PackGetPictureResponse(resp_buf,negative_payload_len,NULL,0,1);
    }
    if (req.pic_id < 0) 
    {
        LOGD("Invalid pic id,ret[-3]\n");
        return PackGetPictureResponse(resp_buf,negative_payload_len,NULL,0,1);
    }
    print_get_picture_request(&req);
    /* 根据id找到对应的文件路径 */
    uint8_t target_id = req.pic_id;
    uint32_t offset = req.offset;
    uint32_t read_len = req.read_len;
    char image_path[PATH_MAX] = {0};
    if (find_file_path_by_id(target_id, image_path) != 0) {
        LOGD("Failed to find file path for pic_id: %u, ret[-4]\n", target_id);
        LOGD("find path : %s\n",image_path);
        return PackGetPictureResponse(resp_buf,negative_payload_len,NULL,0,1);
    }
    LOGD("find path : %s\n",image_path);
    /* 打开文件 */
    FILE *fp = fopen(image_path, "rb");
    if (!fp) 
    {
        LOGD("Failed to open photo file: %s, ret[-5]\n", strerror(errno));
        return PackGetPictureResponse(resp_buf,negative_payload_len,NULL,0,1);
    }

    /* 定位文件指针到指定偏移 */
    if (fseek(fp, offset, SEEK_SET) != 0) 
    {
        LOGD("Failed to seek to offset %lu in file: %s, ret[-6]\n", offset, strerror(errno));
        fclose(fp);
        return PackGetPictureResponse(resp_buf,negative_payload_len,NULL,0,1);
    }

    /* 分配缓冲区并读取数据 */
    uint8_t *data_buffer = malloc(BUFFER_SIZE);
    if (!data_buffer) 
    {
        LOGD("Failed to allocate memory for data buffer, ret[-7]\n");
        fclose(fp);
        return PackGetPictureResponse(resp_buf,negative_payload_len,NULL,0,1);
    }

    // uint16_t bytes_read = fread(data_buffer, 1, read_len, fp);
    // /* 关闭文件 */
    // fclose(fp);

    // /*pic data + 1 result*/
    // uint32_t data_payload_len = bytes_read+1;
    // /* 构建响应 */
    // int ret = PackGetPictureResponse(resp_buf, data_payload_len, data_buffer,bytes_read,0);

    // free(data_buffer); // 释放分配的缓冲区

    bool first_packet = true;
    uint16_t crc_value = 0;
    
    while (!feof(fp))
    {
        size_t bytes_read = fread(data_buffer, 1, BUFFER_SIZE, fp);
        if (bytes_read > 0) 
        {
            crc_value = image_crc16(crc_value, data_buffer, bytes_read); 
            if(first_packet)
            {
                uint32_t data_payload_len = bytes_read + 1; // 可能需要额外加上标识字节
                int ret = PackGetPictureResponse(resp_buf, data_payload_len, data_buffer, bytes_read, 0);
                first_packet = false;  // 标记第一次发送完成
                if (ret < 0) 
                {
                    LOGD("Failed to send initial picture data, ret[%d]\n", ret);
                    break;
                }
            }
            else
            {
                uint32_t data_payload_len = bytes_read; // 可能需要额外加上标识字节
                int ret = send_msg_image(data_buffer, bytes_read);
                if (ret < 0) 
                {
                    LOGD("Failed to send picture data, ret[%d]\n", ret);
                    break;  // 发送失败，退出循环
                }
            }
        }
    }

    /* 发送 CRC 校验值 */
    uint8_t crc_bytes[2] = {crc_value & 0xFF, crc_value >> 8};
    int ret = send_msg_image(crc_bytes, 2);
    if (ret < 0) 
    {
        LOGD("Failed to send CRC value, ret[%d]\n", ret);
    }   

    return 0;
}

void send_picture_data(uint8_t *msg_buf, uint32_t msg_len)
{

}
