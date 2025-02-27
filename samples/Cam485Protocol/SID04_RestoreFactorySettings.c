/*恢复出厂设置*/
#include "cm_common.h"
#include "stdbool.h"
#include "SID04_RestoreFactorySettings.h"
#include "Cam485ProtocolCommon.h"
#include "ImageInfoList.h"
#define LOG_TAG "[SID04_RestoreFactorySettings]"
#include "MsgDispatcher.h"
#include "elog.h"

int SID04_RestoreFactorySettingsCheck(const uint8_t *msg_buf, uint32_t msg_dlc)
{
    int ret = 0;

    (void)msg_buf;
    if (msg_dlc < SID04_MSG_REQ_TOTAL_LEN)  
        ret = -1;

    return ret;
}

static int SID04_BuildMsgHeader(uint8_t *msg_buf, uint32_t msg_dlc)
{
    uint8_t slave_address;
    Get_Protocol_Slave_Address(&slave_address);
    int index = 0;
    msg_buf[index++] = 0xAA;
    msg_buf[index++] = 0x5A;
    msg_buf[index++] = slave_address;
    msg_buf[index++] = 0x04;

    /*填充长度字段*/
    msg_buf[index++] = (uint8_t)(msg_dlc & 0xFF);         // Low byte
    msg_buf[index++] = (uint8_t)((msg_dlc >> 8) & 0xFF);  // 2nd byte
    msg_buf[index++] = (uint8_t)((msg_dlc >> 16) & 0xFF); // 3rd byte
    msg_buf[index++] = (uint8_t)((msg_dlc >> 24) & 0xFF); // High byte
}           

int SID04_RestoreFactorySettings(uint8_t *msg_buf, uint32_t msg_dlc)
{
    LOGD("SID04_RestoreFactorySettings");
    uint8_t resp_buf[SID04_MSG_RESP_TOTAL_LEN];
    memset(resp_buf, 0, SID04_MSG_RESP_TOTAL_LEN);
    int ret = 0;
    uint8_t result = 0;
    uint8_t id;
    ret = SID04_RestoreFactorySettingsCheck(msg_buf, msg_dlc);
    if (ret != 0)
    {
        return -1;
    }
    
    SID04_BuildMsgHeader(resp_buf, SID04_MSG_RESP_DATA_LEN);
    resp_buf[8] = result;
    //printf("After header: msg_buf[8] = 0x%02X, msg_buf[9] = 0x%02X\n", resp_buf[8], resp_buf[9]);
    send_msg_resp(resp_buf, SID04_MSG_RESP_TOTAL_LEN);

    system("cp /system/bin/cm_config.ini /system/etc/cm_config.ini -f");
    system("rm /system/md5.ini");
    usleep(1000*10);
    system("reboot");
}
