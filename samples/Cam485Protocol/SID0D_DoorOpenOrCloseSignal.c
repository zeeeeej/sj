/*主动拍照*/
#include "cm_common.h"

#include "stdbool.h"
#include "SID0D_DoorOpenOrCloseSignal.h"
#include "Cam485ProtocolCommon.h"
#include "ImageInfoList.h"
    
int SID0D_DoorOpenOrCloseSignalCheck(const uint8_t *msg_buf, uint32_t msg_dlc)
{
    int ret = 0;

    (void)msg_buf;
    if (msg_dlc < SID0D_MSG_REQ_TOTAL_LEN)
        ret = -1;

    return ret;
}

static int SID0D_BuildMsgHeader(uint8_t *msg_buf, uint32_t msg_dlc)
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

int SID0D_DoorOpenOrCloseSignal(uint8_t *msg_buf, uint32_t msg_dlc)
{
    log_i("SID0D_DoorOpenOrCloseSignal");
    if(msg_buf == NULL)
    {
        log_e("error param");
    }
    int ret = 0 ;
    uint8_t door_status;
    ret = SID0D_DoorOpenOrCloseSignalCheck(msg_buf,msg_dlc);
    if(ret != 0)
    {
        log_e("msg error");
    }
    uint8_t resp_buf[SID0D_MSG_RESP_TOTAL_LEN] = {0};
    SID0D_BuildMsgHeader(resp_buf,SID0D_MSG_RESP_DATA_LEN);

    door_status = query_door_status();
    if(door_status == DOOR_CLOSED)
    {
        log_i("door close now , door_status : %d\n",door_status);
    }
    else
    {
        log_i("door open now , door_status : %d\n",door_status);
    }
    resp_buf[8] = door_status;

    
    ret = send_msg_resp(resp_buf,SID0D_MSG_RESP_TOTAL_LEN);
    if(ret == 0)
    {
        log_e("send scuess");
    }
    else
    {
        log_i("send fail");
    }
}
