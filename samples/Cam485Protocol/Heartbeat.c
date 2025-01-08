#include "Heartbeat.h"
#include "cm_common.h"
int SID01_HeartBeat(uint8_t *msg_buf, uint16_t msg_dlc)
{
    int ret = SID01_CheckHeartBeat(msg_buf, msg_dlc);
    if(ret < 0)
    {
        return -1;
    }

    //合法，则构建回复数据帧
    msg_buf[4] = (msg_buf[4] + 1) & 0xFF;
    msg_buf[5] = 0x00;

    return 0;
}

/// @brief 检查心跳包
/// @param msg_buf 数据缓冲区
/// @param msg_dlc 数据长度
/// @return 
int SID01_CheckHeartBeat(const uint8_t *msg_buf, uint16_t msg_dlc)
{
    LOGD("SID01_CheckHeartBeat\n");
    if(msg_dlc != HEART_FRAME_SIZE)
    {
        LOGD("SID01_CheckHeartBeat: msg_dlc error\n");
        return -1;
    }
     if (msg_buf[0] != 0xAA || msg_buf[1] != 0x5A) {
        LOGD("SID01_CheckHeartBeat: header error\n");
        return -2; 
    }

    if (msg_buf[2] != CMD_HEARTBEAT || msg_buf[3] != SUBCMD_HEARTBEAT) {
        LOGD("SID01_CheckHeartBeat: command error\n");
        return -3;
    }

    for (int i = 5; i < 9; i++) 
    {
        if (msg_buf[i] != 0x00) 
        {
            LOGD("SID01_CheckHeartBeat: reserved error\n");
            return -4; 
        }
    }
    return 0;
}
