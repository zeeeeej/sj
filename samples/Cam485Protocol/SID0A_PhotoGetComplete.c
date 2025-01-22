/*主动拍照*/
#include "cm_common.h"
#include "stdbool.h"
#include "SID0A_PhotoGetComplete.h"
#include "ProtocolPort.h"
#include "ImageInfoList.h"


int SID0A_PhotoGetCompleteCheck(const uint8_t *msg_buf, uint32_t msg_dlc)
{
    int ret = 0;

    (void)msg_buf;
    if (msg_dlc < SID0A_MSG_REQ_TOTAL_LEN)
        ret = -1;

    return ret;
}

static int SID0A_BuildMsgHeader(uint8_t *msg_buf, uint32_t msg_dlc)
{
    int index = 0;
    msg_buf[index++] = 0xAA;
    msg_buf[index++] = 0x5A;
    msg_buf[index++] = SLAVE_ADDR;
    msg_buf[index++] = 0x05;

    /*填充长度字段*/
    msg_buf[index++] = (uint8_t)(msg_dlc & 0xFF);         // Low byte
    msg_buf[index++] = (uint8_t)((msg_dlc >> 8) & 0xFF);  // 2nd byte
    msg_buf[index++] = (uint8_t)((msg_dlc >> 16) & 0xFF); // 3rd byte
    msg_buf[index++] = (uint8_t)((msg_dlc >> 24) & 0xFF); // High byte
}           

int SID0A_PhotoGetComplete(uint8_t *msg_buf, uint32_t msg_dlc)
{
    LOGD("SID0A_PhotoGetComplete");
    uint8_t resp_buf[SID0A_MSG_RESP_TOTAL_LEN];
    memset(resp_buf, 0, SID0A_MSG_RESP_TOTAL_LEN);
    int ret = 0;
    uint8_t result = 0;
    uint8_t id;
    ret = SID0A_PhotoGetCompleteCheck(msg_buf, msg_dlc);
    if (ret != 0)
    {
        return -1;
    }
    
    SID0A_BuildMsgHeader(resp_buf, SID0A_MSG_RESP_DATA_LEN);
    resp_buf[8] = result;

    send_msg_resp(resp_buf, SID0A_MSG_RESP_TOTAL_LEN);
}
