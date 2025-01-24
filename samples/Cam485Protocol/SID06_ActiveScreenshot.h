#ifndef SID06_ACTIVESCREENSHOT
#define SID06_ACTIVESCREENSHOT

#include <stdio.h>
#include <stdint.h>

#define SID06_MSG_REQ_TOTAL_LEN      10 
#define SID06_MSG_RESP_DATA_LEN      2
#define SID06_MSG_RESP_TOTAL_LEN    (10+SID06_MSG_RESP_DATA_LEN)

int SID06_ActiveScreenshot(uint8_t *msg_buf, uint32_t msg_dlc);
#endif