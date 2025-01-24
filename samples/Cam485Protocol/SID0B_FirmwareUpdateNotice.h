#ifndef SID0B_FIRMWAREUPDATENOTICE  
#define SID0B_FIRMWAREUPDATENOTICE
#include <stdio.h>
#include <stdint.h>
#include "SID0BC_FirmwareUpdateInfo.h"
#include "MsgDispatcher.h"

#define SID0B_MSG_REQ_TOTAL_LEN      (30) 
#define SID0B_MSG_RESP_DATA_LEN      (5)
#define SID0B_MSG_RESP_TOTAL_LEN     (10+SID0B_MSG_RESP_DATA_LEN)


int SID0B_FirmwareUpdateNotice(uint8_t *msg_buf, uint32_t msg_dlc);
#endif