/*主动拍照*/
#include "cm_common.h"

#include "stdbool.h"
#include "SID0C_SendFirmwareUpdatePacket.h"
#include "ProtocolPort.h"
#include "ImageInfoList.h"


// static UpdatePacketStruct  UpdatePacketInfo;

int SID0C_FirmwareUpdateNoticeCheck(const uint8_t *msg_buf, uint32_t msg_dlc)
{
    int ret = 0;

    (void)msg_buf;
    // if (msg_dlc != SID0C_MSG_REQ_TOTAL_LEN)
    //     ret = -1;
    if (msg_dlc < ((msg_buf[4]) | (msg_buf[5] << 8) | (msg_buf[6] << 16) | (msg_buf[7] << 24) ))
        ret = -1;

    return ret;
}

static int SID0C_BuildMsgHeader(uint8_t *msg_buf, uint32_t msg_dlc)
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

// static UpdatePacketStruct * GetSystemAllowUpdateFlag()
// {
//     UpdatePacketStruct *pUpdatePacketInfo = &UpdatePacketInfo;
//     pUpdatePacketInfo->updadeable = 0;
//     pUpdatePacketInfo->offset = 0;
//     return pUpdatePacketInfo;
// }

int SID0C_SendFirmwareUpdatePacket(uint8_t *msg_buf, uint32_t msg_dlc)
{
    LOGD("SID0C_SendFirmwareUpdatePacket\n");
    uint8_t resp_buf[SID0C_MSG_RESP_TOTAL_LEN];
    memset(resp_buf, 0, SID0C_MSG_RESP_TOTAL_LEN);
    int ret = 0;
    uint8_t result = 0;
    uint8_t id;
    ret = SID0C_FirmwareUpdateNoticeCheck(msg_buf, msg_dlc);
    if (ret != 0)
    {
        LOGD("SID0C_FirmwareUpdateNoticeCheck ERROR\n");
        return -1;
    }

    /*读取数据*/

    /*打印数据*/
    uint32_t len = msg_buf[4] + (msg_buf[5]<<8)+(msg_buf[6]<<16)+(msg_buf[7]<<24);
    uint32_t offset = msg_buf[8] + (msg_buf[9]<<8)+(msg_buf[10]<<16)+(msg_buf[11]<<24);
    uint32_t dataLen = msg_buf[12] + (msg_buf[13]<<8)+(msg_buf[14]<<16)+(msg_buf[15]<<24);

    for(int i=0;i<len;i++)
    {
        LOGD("%02X ",msg_buf[i+8]);
    }
    LOGD("\n");
    
    
    
    SID0C_BuildMsgHeader(resp_buf, SID0C_MSG_RESP_DATA_LEN);

    // UpdatePacketStruct *pUpdatePacketInfo = GetSystemAllowUpdateFlag();
    // resp_buf[8] = pUpdatePacketInfo->updadeable;
    // resp_buf[9] = pUpdatePacketInfo->offset & 0xff;
    // resp_buf[10] = (pUpdatePacketInfo->offset >> 8) & 0xff;
    // resp_buf[11] = (pUpdatePacketInfo->offset >> 16) & 0xff;
    // resp_buf[12] = (pUpdatePacketInfo->offset >> 24) & 0xff;


    send_msg_resp(resp_buf, SID0C_MSG_RESP_TOTAL_LEN);
}

