#ifndef SID0D_DOOROPENORCLOSESIGNAL
#define SID0D_DOOROPENORCLOSESIGNAL
#include <stdio.h>
#include <stdint.h>
#include <elog.h>

#define SID0D_MSG_REQ_TOTAL_LEN      (10) 
#define SID0D_MSG_RESP_DATA_LEN      (1)
#define SID0D_MSG_RESP_TOTAL_LEN     (10+SID0D_MSG_RESP_DATA_LEN)




int SID0D_DoorOpenOrCloseSignal(uint8_t *msg_buf, uint32_t msg_dlc);
#endif