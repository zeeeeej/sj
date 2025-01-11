// author :li

#include "CommonMsgTable.h"
#include "Heartbeat.h"
// 服务配置表
const MsgService MsgServiceList[SID_NUM]  =
{
    {SID_01, SID01_HeartBeat},
};
