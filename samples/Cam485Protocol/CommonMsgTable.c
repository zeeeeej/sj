// author :li

#include "CommonMsgTable.h"
#include "Heartbeat.h"
#include "SID06_ActiveScreenshot.h"
#include "SendPhoto.h"
#include "SID07_QueryPhotoInfo.h"
#include "SID09_GetPhoto.h"
// 服务配置表
const MsgService MsgServiceList[SID_NUM]  =
{
    {SID_01, SID01_HeartBeat},
    {SID_06,SID06_ActiveScreenshot},
    {SID_08,HandleDeletePhoto},
    {SID_07, SID07_QueryPhotoInfo},
    {SID_09,SID09_GetPhoto}
};
