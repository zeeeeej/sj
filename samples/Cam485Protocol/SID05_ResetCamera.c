/*重启摄像头*/
#include "cm_common.h"
#include "stdbool.h"
#include "SID05_ResetCamera.h"
#include "ProtocolPort.h"
#include "ImageInfoList.h"



int SID05_ResetCameraCheck(const uint8_t *msg_buf, uint32_t msg_dlc)
{
    int ret = 0;

    (void)msg_buf;
    if (msg_dlc < SID05_MSG_REQ_TOTAL_LEN)
        ret = -1;

    return ret;
}

static int SID05_BuildMsgHeader(uint8_t *msg_buf, uint32_t msg_dlc)
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

int SID05_ResetCamera(uint8_t *msg_buf, uint32_t msg_dlc)
{
    LOGD("SID05_ResetCamera");
    uint8_t resp_buf[SID05_MSG_RESP_TOTAL_LEN];
    memset(resp_buf, 0, SID05_MSG_RESP_TOTAL_LEN);
    int ret = 0;
    uint8_t result = 0;
    uint8_t id;
    ret = SID05_ResetCameraCheck(msg_buf, msg_dlc);
    if (ret != 0)
    {
        return -1;
    }
    
    SID05_BuildMsgHeader(resp_buf, SID05_MSG_RESP_DATA_LEN);
    resp_buf[8] = result;

    send_msg_resp(resp_buf, SID05_MSG_RESP_TOTAL_LEN);
    usleep(1000*10);
    system("reboot");
}
