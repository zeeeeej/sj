#ifndef SID04_RESTOREFACTORYSETTINGS
#define SID04_RESTOREFACTORYSETTINGS

#include <stdio.h>
#include <stdint.h>

#define SID04_MSG_REQ_TOTAL_LEN      10 
#define SID04_MSG_RESP_DATA_LEN      1
#define SID04_MSG_RESP_TOTAL_LEN    (10+SID04_MSG_RESP_DATA_LEN)

int SID04_RestoreFactorySettings(uint8_t *msg_buf, uint32_t msg_dlc);
#endif
