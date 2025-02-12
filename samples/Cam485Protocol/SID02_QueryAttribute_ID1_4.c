/*查询属性*/
#include "cm_common.h"
#include "stdbool.h"
#include "SID02_QueryAttribute.h"
#include "Cam485ProtocolCommon.h"
#include "ImageInfoList.h"

static int SID0B_WriteDataToBuf(uint8_t *msg_buf, uint32_t index,uint32_t data)
{
    /*填充长度字段*/
    msg_buf[index++] = (uint8_t)(data & 0xFF);         // Low byte
    msg_buf[index++] = (uint8_t)((data >> 8) & 0xFF);  // 2nd byte
    msg_buf[index++] = (uint8_t)((data >> 16) & 0xFF); // 3rd byte
    msg_buf[index++] = (uint8_t)((data >> 24) & 0xFF); // High byte
}

static int SID0B_BuildMsgHeader(uint8_t *msg_buf, uint32_t msg_dlc)
{
    int index = 0;
    msg_buf[index++] = 0xAA;
    msg_buf[index++] = 0x5A;
    msg_buf[index++] = SLAVE_ADDR;
    msg_buf[index++] = 0x02;

    /*填充长度字段*/
    msg_buf[index++] = (uint8_t)(msg_dlc & 0xFF);         // Low byte
    msg_buf[index++] = (uint8_t)((msg_dlc >> 8) & 0xFF);  // 2nd byte
    msg_buf[index++] = (uint8_t)((msg_dlc >> 16) & 0xFF); // 3rd byte
    msg_buf[index++] = (uint8_t)((msg_dlc >> 24) & 0xFF); // High byte
}

int Handle_SID02_Attribute_SloveAddress(void)
{
    uint8_t result = 0;
    uint8_t slave_address = Get_slave_address();
    LOGD("slave_address=%u\n",slave_address);
    uint8_t resp_buf[13];
    memset(resp_buf, 0, sizeof(resp_buf));
    SID0B_BuildMsgHeader(resp_buf, 3);
    resp_buf[8] = 1;
    resp_buf[9] = result;
    resp_buf[10] = slave_address;
    send_msg_resp(resp_buf, 13);
    return 0;
}

int Handle_SID02_Attribute_FirmwareVersion(void)
{
    uint8_t result = 0;
    uint32_t firwareversion = Get_Firwareversion();
    LOGD("firwareversion=%u\n",firwareversion);
    uint8_t resp_buf[10+2+4];
    memset(resp_buf, 0, sizeof(resp_buf));
    SID0B_BuildMsgHeader(resp_buf, 2+4);
    resp_buf[8] = 2;
    resp_buf[9] = result;
    SID0B_WriteDataToBuf(resp_buf,10,firwareversion);
    send_msg_resp(resp_buf, 10+2+4);
    return 0;
}

int Handle_SID02_Attribute_CameraNum(void)
{
    uint8_t result = 0;
    uint32_t firwareversion = Get_CameraNum();
    LOGD("firwareversion=%u\n",firwareversion);
    uint8_t resp_buf[10+2+4];
    memset(resp_buf, 0, sizeof(resp_buf));
    SID0B_BuildMsgHeader(resp_buf, 2+4);
    resp_buf[8] = 2;
    resp_buf[9] = result;
    SID0B_WriteDataToBuf(resp_buf,10,firwareversion);
    send_msg_resp(resp_buf, 10+2+4);
    return 0;
}

int Handle_SID02_Attribute_Baudrate(void)
{
    uint8_t result = 0;
    uint32_t Rs485Baudrate = Get_Rs485Baudrate();
    LOGD("Rs485Baudrate=%u\n",Rs485Baudrate);
    uint8_t resp_buf[10+2+4];
    memset(resp_buf, 0, sizeof(resp_buf));
    SID0B_BuildMsgHeader(resp_buf, 2+4);
    resp_buf[8] = 2;
    resp_buf[9] = result;
    SID0B_WriteDataToBuf(resp_buf,10,Rs485Baudrate);
    send_msg_resp(resp_buf, 10+2+4);
    return 0;
}