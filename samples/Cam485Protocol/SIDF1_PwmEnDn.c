/*测试指令
    加热换开启关闭*/
#define LOG_TAG "[SIDFF_TestChannel]"
#include "SID01_Heartbeat.h"
#include "cm_common.h"
#include <stdint.h>
#include "MsgDispatcher.h"
#include "Cam485ProtocolCommon.h"


#include "elog.h"

int SIDF1_PwmEnDn(uint8_t *msg_buf, uint32_t msg_dlc)
{
    uint8_t EnDn = msg_buf[8];
    if(EnDn == 0)
    {
        /*加热换关闭*/
        stop_heating();
    }
    else
    {
        /*加热换开启*/
        start_heating();
    }
    return 0;
}


