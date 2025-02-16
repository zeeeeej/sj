/*测试用*/

#include "SID01_Heartbeat.h"
#include "cm_common.h"
#include <stdint.h>
#include "MsgDispatcher.h"
#include "elog.h"
#define LOG_TAG "[SIDFF_TestChannel]"
int SIDFF_TestChannel(uint8_t *msg_buf, uint32_t msg_dlc)
{

    log_i("SIDFF_TestChannel");
    log_i("msg_dlc: %d", msg_dlc);
    int index = 0;
    int seq = 0;
    char rsp_buf[64] = {0};
    //合法，则构建回复数据帧
    rsp_buf[index++] = 0xAA;
    rsp_buf[index++] = 0x5A;
    rsp_buf[index++] = 0x01;
    rsp_buf[index++] = 0x01;

    rsp_buf[index++] = 0x01;
    rsp_buf[index++] = 0x00;
    rsp_buf[index++] = 0x00;
    rsp_buf[index++] = 0x00;

    seq = msg_buf[8];
    seq = seq+1;
    if(seq == 255)
    {
        seq = 0;
    }
    rsp_buf[index++] = seq;
    rsp_buf[index++] = 0x7F;
    rsp_buf[index++] = 0xEA;

    send_msg_resp(rsp_buf, 11);
    return 0;
}


