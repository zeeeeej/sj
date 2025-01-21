#ifndef SID0A_PHOTOGETCOMPLETE  
#define SID0A_PHOTOGETCOMPLETE

#include <stdio.h>
#include <stdint.h>

#define SID0A_MSG_REQ_TOTAL_LEN      (10) 
#define SID0A_MSG_RESP_DATA_LEN      (1)
#define SID0A_MSG_RESP_TOTAL_LEN     (SID0A_MSG_REQ_TOTAL_LEN + SID0A_MSG_RESP_DATA_LEN)


int SID0A_PhotoGetComplete(uint8_t *msg_buf, uint32_t msg_dlc);
#endif