#ifndef SID05_RESETCAMERA
#define SID05_RESETCAMERA

#include <stdio.h>
#include <stdint.h>

#define SID05_MSG_REQ_TOTAL_LEN      (10) 
#define SID05_MSG_RESP_DATA_LEN      (1)
#define SID05_MSG_RESP_TOTAL_LEN     (SID05_MSG_REQ_TOTAL_LEN + SID05_MSG_RESP_DATA_LEN)

int SID05_ResetCamera(uint8_t *msg_buf, uint32_t msg_dlc);
#endif