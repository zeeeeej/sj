


#include "SID07_QueryPhotoInfo.h"
#include "Cam485ProtocolCommon.h"
#include "ImageInfoList.h"
#include "cm_common.h"
#include "elog.h"

#include "stdbool.h"
#include "string.h"


#define TAG_NAME  "[SID07->QUERY PHOTO]"
static int SID07_BuildMsgHeader(uint8_t *msg_buf, uint32_t msg_dlc)
{
    int index = 0;
    msg_buf[index++] = 0xAA;
    msg_buf[index++] = 0x5A;
    msg_buf[index++] = SLAVE_ADDR;
    msg_buf[index++] = 0x07;

    /*填充长度字段*/
    msg_buf[index++] = (uint8_t)(msg_dlc & 0xFF);         // Low byte
    msg_buf[index++] = (uint8_t)((msg_dlc >> 8) & 0xFF);  // 2nd byte
    msg_buf[index++] = (uint8_t)((msg_dlc >> 16) & 0xFF); // 3rd byte
    msg_buf[index++] = (uint8_t)((msg_dlc >> 24) & 0xFF); // High byte
}


int SID07_QueryPhotoInfo(uint8_t *msg_buf, uint32_t msg_dlc)
{
    log_i("quert photo");

    /*
        仅用作测试回包消息
     */
    // DataNode node1 = { 1, 0x01, 0x10, 1234567890, 1024, "md5value1", "/path/to/image1.jpg" };
    // appendToActiveTriggerList(node1);


    /*info缓冲区*/
    uint8_t* image_info_buf;
    /*image info的个数*/
    uint8_t image_info_count;
    image_info_buf = copy_both_lists_to_buffer(&image_info_count);
    /*info缓冲区长度*/
    uint32_t image_info_buf_len = image_info_count*sizeof(BufferedDataNode);
    log_i("total pic info count : %d",image_info_count);
    log_d("image info buf total len : %d",image_info_buf_len);


    printAllListlist();
    /*8header + data(info+info_num) + 2crc*/
    uint8_t resp_buf[8+image_info_buf_len+1+2];

    uint32_t data_payload_len = image_info_buf_len + 1;
    SID07_BuildMsgHeader(resp_buf ,data_payload_len);

    /*填充info个数*/
    resp_buf[8] = image_info_count;

    /*回复消息帧总长度*/
    uint32_t total_length = data_payload_len + 8 + 2;
    memcpy(resp_buf+9 , image_info_buf ,image_info_buf_len);
    free(image_info_buf);
    send_msg_resp(resp_buf,total_length);
}