/*主动拍照*/
#include "cm_common.h"

#include "stdbool.h"
#include "SID0B_FirmwareUpdateNotice.h"
#include "ProtocolPort.h"
#include "ImageInfoList.h"

static UpdatePacketStruct  UpdatePacketInfo;

int SID0B_FirmwareUpdateNoticeCheck(const uint8_t *msg_buf, uint32_t msg_dlc)
{
    int ret = 0;

    (void)msg_buf;
    if (msg_dlc != SID0B_MSG_REQ_TOTAL_LEN)
        ret = -1;

    return ret;
}

static int SID0B_BuildMsgHeader(uint8_t *msg_buf, uint32_t msg_dlc)
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

static UpdatePacketStruct * GetSystemAllowUpdateFlag()
{
    UpdatePacketStruct *pUpdatePacketInfo = &UpdatePacketInfo;
    pUpdatePacketInfo->updadeable = 0;
    pUpdatePacketInfo->offset = 0;
    return pUpdatePacketInfo;
}

int SID0B_FirmwareUpdateNotice(uint8_t *msg_buf, uint32_t msg_dlc)
{
    LOGD("SID0B_FirmwareUpdateNotice\n");
    uint8_t resp_buf[SID0B_MSG_RESP_TOTAL_LEN];
    memset(resp_buf, 0, SID0B_MSG_RESP_TOTAL_LEN);
    int ret = 0;
    uint8_t result = 0;
    uint8_t id;
    ret = SID0B_FirmwareUpdateNoticeCheck(msg_buf, msg_dlc);
    if (ret != 0)
    {
        LOGD("SID0B_FirmwareUpdateNoticeCheck\n");
        return -1;
    }
    
    SID0B_BuildMsgHeader(resp_buf, SID0B_MSG_RESP_DATA_LEN);

    UpdatePacketStruct *pUpdatePacketInfo = GetSystemAllowUpdateFlag();
    resp_buf[8] = pUpdatePacketInfo->updadeable;
    resp_buf[9] = pUpdatePacketInfo->offset & 0xff;
    resp_buf[10] = (pUpdatePacketInfo->offset >> 8) & 0xff;
    resp_buf[11] = (pUpdatePacketInfo->offset >> 16) & 0xff;
    resp_buf[12] = (pUpdatePacketInfo->offset >> 24) & 0xff;


    send_msg_resp(resp_buf, SID0B_MSG_RESP_TOTAL_LEN);
}
