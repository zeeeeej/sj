#define LOG_TAG "[SID09->GET PHOTO]"
#include "stdio.h"
#include "malloc.h"
#include <string.h>  // 包含 strerror 声明
#include <errno.h>   // 包含 errno 定义
#include <limits.h>  // 包含 PATH_MAX 定义
#include <stdbool.h> // 包含 bool 定义
#include <unistd.h>
#include "cm_common.h"
#include "SID09_GetPhoto.h"
#include "Cam485ProtocolCommon.h"
#include "MsgDispatcher.h"
#include "ImageInfoList.h"
#include "elog.h"

#define IMG_BUFFER_SIZE (8000)
#define RESP_BUF_SIZE (IMG_BUFFER_SIZE + 50)


static char resp_buf[RESP_BUF_SIZE];
static char img_data_buf[IMG_BUFFER_SIZE];
static int send_count = 0;
#ifndef MIN
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif
static int ParseGetPictureRequest(const uint8_t *msg_buf, uint32_t msg_len, GetPictureRequest_t *req)
{
    if (msg_buf == NULL || req == NULL || msg_len < 19)
    {
        LOGD("error:msg_len = %d", msg_len);
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



static int send_pic_finish_resp(uint8_t* msg_buf,uint8_t result)
{
    uint8_t slave_address;
    Get_Protocol_Slave_Address(&slave_address);
    int index = 0;
    msg_buf[index++] = 0xAA;
    msg_buf[index++] = 0x5A;
    msg_buf[index++] = slave_address;
    msg_buf[index++] = 0x0A;

    uint32_t data_payload_len = 1;
    /*填充长度字段*/
    msg_buf[index++] = (uint8_t)(data_payload_len & 0xFF);         // Low byte
    msg_buf[index++] = (uint8_t)((data_payload_len >> 8) & 0xFF);  // 2nd byte
    msg_buf[index++] = (uint8_t)((data_payload_len >> 16) & 0xFF); // 3rd byte
    msg_buf[index++] = (uint8_t)((data_payload_len >> 24) & 0xFF); // High byte
    msg_buf[index++] = result;
    /*crc(2) + data payload(1) + header(8)*/
    uint32_t total_len = data_payload_len + 8+2;
    return send_msg_resp(msg_buf, total_len);
}


/**
 * 打印 GetPictureRequest_t 结构体的内容
 * @param request 指向 GetPictureRequest_t 结构体的指针
 */
void print_get_picture_request(const GetPictureRequest_t *request)
{
    if (request == NULL)
    {
        printf("Invalid request pointer.\n");
        return;
    }

    printf("GetPictureRequest_t contents:\n");
    printf("Picture ID: %u\n", request->pic_id);
    printf("Offset: %u\n", request->offset);
    printf("Read Length: %u\n", request->read_len);
    printf("Result: %d\n", request->result);
}




int PackGetPictureResponseHeader(uint8_t *msg_buf, uint32_t data_payload_len, uint8_t result)
{
    uint8_t slave_address;
    Get_Protocol_Slave_Address(&slave_address);
    int index = 0;
    msg_buf[index++] = 0xAA;
    msg_buf[index++] = 0x5A;
    msg_buf[index++] = slave_address;
    msg_buf[index++] = 0x09;

    /*填充长度字段*/
    msg_buf[index++] = (uint8_t)(data_payload_len & 0xFF);         // Low byte
    msg_buf[index++] = (uint8_t)((data_payload_len >> 8) & 0xFF);  // 2nd byte
    msg_buf[index++] = (uint8_t)((data_payload_len >> 16) & 0xFF); // 3rd byte
    msg_buf[index++] = (uint8_t)((data_payload_len >> 24) & 0xFF); // High byte

    msg_buf[index++] = result;
    elog_hexdump("get photo header", 16, msg_buf, 9);
    return 0;
    // if (pic_data == NULL)
    // {
    //     msg_buf[index++] = result;
    //     send_msg_image(resp_buf, data_payload_len + 8);
    // }
    // else
    // {
    //     msg_buf[index++] = result;
    //     memcpy(msg_buf + index, pic_data, pic_data_len);
    //     send_msg_image(resp_buf, data_payload_len + 8);
    // }
}


static int send_negative_resp(uint8_t *msg_buf)
{   
    int ret = 0; 
    int negative_result = 1;
    int negative_payload_len = 1;
    int negative_resp_total_len = negative_payload_len + 8 + 2;
    PackGetPictureResponseHeader(msg_buf,negative_payload_len,negative_result);
    ret = send_msg_resp(msg_buf,negative_resp_total_len);
    ret = send_pic_finish_resp(msg_buf,negative_result);
    return ret;
}




int SID09_GetPhoto(uint8_t *msg_buf, uint32_t msg_len)
{
    log_i("SID09_GetPhoto , Count : %d",send_count);
    send_count++;
    GetPictureRequest_t req;
    int negative_payload_len = 1;
    int res = ParseGetPictureRequest(msg_buf, msg_len, &req);
    if (0 != res)
    {
        log_e("Failed to parse photo request,ret[%d]", res);
        return send_negative_resp(resp_buf);
    }
    if (req.command != 0x09)
    {
        log_e("Invalid command,ret[-2]");
        return send_negative_resp(resp_buf);
    }
    if (req.pic_id < 0)
    {
        log_e("Invalid pic id,ret[-3]");
        return send_negative_resp(resp_buf);
    }
    print_get_picture_request(&req);
    /* 根据id找到对应的文件路径 */
    uint8_t target_id = req.pic_id;
    uint32_t read_offset = req.offset;
    uint32_t read_len = req.read_len;
    char image_path[PATH_MAX] = {0};
    if (find_file_path_by_id(target_id, image_path) != 0)
    {
        log_e("Failed to find file path for pic_id: %u, ret[-4]", target_id);
        log_e("find path : %s", image_path);
        return send_negative_resp(resp_buf);
    }
    log_i("find path : %s\n", image_path);
    /* 打开文件 */
    FILE *fp = fopen(image_path, "rb");
    if (!fp)
    {
        log_e("Failed to open photo file: %s, ret[-5]\n", strerror(errno));
        return send_negative_resp(resp_buf);
    }

    /* 定位文件指针到指定偏移 */
    if (fseek(fp, read_offset, SEEK_SET) != 0)
    {
        LOGD("Failed to seek to offset %lu in file: %s, ret[-6]\n", read_offset, strerror(errno));
        fclose(fp);
        return send_negative_resp(resp_buf);
    }

    /* 分配缓冲区并读取数据 */
    // uint8_t *data_buffer = malloc(BUFFER_SIZE);
    uint8_t *data_buffer = img_data_buf;
    if (!data_buffer)
    {
        LOGD("Failed to allocate memory for data buffer, ret[-7]\n");
        fclose(fp);
        return send_negative_resp(resp_buf);
    }





    // 在打开文件之后立即获取文件大小
    fseek(fp, 0L, SEEK_END);
    uint32_t file_size = ftell(fp);
    rewind(fp); // 将文件指针重置到起始位置
    fseek(fp, read_offset, SEEK_SET); // 再次定位到指定偏移

    /*校验offset和reradLen的合理性*/
    if (read_offset >= file_size || read_len > file_size - read_offset)
    {
        log_e("Invalid read range, ret[-7]");
        fclose(fp);
        return send_negative_resp(resp_buf);
    }
    uint32_t data_payload_len = file_size + 1;
    log_i("data payload len : %d", data_payload_len);
    log_i("pic len : %d", file_size);
    bool first_packet = true;
    uint16_t crc_value = 0;
    uint32_t total_sent = 0; // 记录总共已经发送了多少字节
    int send_success_flag = 0;
    int ret;

    while (total_sent < read_len && !feof(fp))
    {
        size_t bytes_to_read = MIN(IMG_BUFFER_SIZE, read_len - total_sent);
        size_t bytes_read = fread(data_buffer, 1, bytes_to_read, fp);
        if (bytes_read > 0)
        {
            
            total_sent += bytes_read; // 更新已发送的总字节数

            if (first_packet)
            {
                /* 构建帧头 */
                PackGetPictureResponseHeader(resp_buf, data_payload_len, 0);
                /* 发送帧头 */
                int frame_header_len = 1 + 8;
                ret = send_msg_image(resp_buf, frame_header_len);
                crc_value = image_crc16(crc_value, resp_buf, frame_header_len);
                first_packet = false; // 标记第一次发送完成
                if (ret < 0)
                {
                    log_e("Failed to send initial picture data, ret[%d]", ret);
                    send_success_flag = -1;
                    break;
                }
                /* 再发送IMG_BUFFER_SIZE个字节的像素数据 */
                ret = send_msg_image(data_buffer, bytes_read);
                if (ret < 0)
                {
                    log_e("Failed to send picture data, ret[%d]", ret);
                    send_success_flag = -1;
                    break; // 发送失败，退出循环
                }
                crc_value = image_crc16(crc_value, data_buffer, bytes_read);
            }
            else
            {
                ret = send_msg_image(data_buffer, bytes_read);
                if (ret < 0)
                {
                    log_e("Failed to send picture data, ret[%d]", ret);
                    send_success_flag = -1;
                    break; // 发送失败，退出循环
                }
                crc_value = image_crc16(crc_value, data_buffer, bytes_read);
            }

            // 打印进度信息（显示已发送的字节数和总字节数）
            printf("Sending... Sent %u bytes out of %u\n", total_sent, read_len);

            usleep(1000); // 暂停1毫秒以减少CPU占用率
        }
    }

    if (send_success_flag == 0)
    {
        // 发送 CRC 校验值
        uint8_t crc_bytes[2] = {crc_value & 0xFF, crc_value >> 8};
        ret = send_msg_image(crc_bytes, 2);
        if (ret < 0)
        {
            log_e("Failed to send CRC value, ret[%d]", ret);
        }
        /*主动发送图片完成帧*/
        ret =  send_pic_finish_resp(resp_buf,0);
        if(ret < 0)
        {
            log_e("Failed to send pic finish frame, ret[%d]", ret);
        }
    }
    log_i("GET PHOTO END");
    // free(data_buffer); // 释放分配的缓冲区
    fclose(fp);

    return 0;
}


