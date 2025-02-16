
#include "CommonMsgTable.h"
#include "SID01_Heartbeat.h"
#include "SID02_QueryAttribute.h"
#include "SID03_SetAttribute.h"
#include "SID04_RestoreFactorySettings.h"
#include "SID05_ResetCamera.h"
#include "SID06_ActiveScreenshot.h"
#include "SID08_DeletePicture.h"
#include "SID07_QueryPhotoInfo.h"
#include "SID09_GetPhoto.h"
#include "SID0A_PhotoGetComplete.h"
#include "SID0B_FirmwareUpdateNotice.h"
#include "SID0C_SendFirmwareUpdatePacket.h"
#include "SID0D_DoorOpenOrCloseSignal.h"
#include "SIDFF_TestChannel.h"
// 服务配置表
const MsgService MsgServiceList[SID_NUM]  =
{
    {SID_01, SID01_HeartBeat},
    {SID_02, SID02_QueryAttribute},
    {SID_03, SID03_SetAttribute},
    {SID_04, SID04_RestoreFactorySettings},
    {SID_05, SID05_ResetCamera},
    {SID_06, SID06_ActiveScreenshot},
    {SID_08, SID08_HandleDeletePhoto},
    {SID_07, SID07_QueryPhotoInfo},
    {SID_08, SID08_HandleDeletePhoto},
    {SID_09, SID09_GetPhoto},
    {SID_0A, SID0A_PhotoGetComplete},
    {SID_0B, SID0B_FirmwareUpdateNotice},
    {SID_0C, SID0C_SendFirmwareUpdatePacket},
    {SID_0D, SID0D_DoorOpenOrCloseSignal},
    {SID_FF, SIDFF_TestChannel},    
};
