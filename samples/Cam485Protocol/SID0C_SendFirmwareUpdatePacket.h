#ifndef SID0C_SENDFIRMWAREUPDATEPACKET
#define SID0C_SENDFIRMWAREUPDATEPACKET
#include <stdio.h>
#include <stdint.h>

#define SID0C_MSG_REQ_TOTAL_LEN      (30) 
#define SID0C_MSG_RESP_DATA_LEN      (5)
#define SID0C_MSG_RESP_TOTAL_LEN     (10+SID0C_MSG_RESP_DATA_LEN)

int SID0C_SendFirmwareUpdatePacket(uint8_t *msg_buf, uint32_t msg_dlc);
#endif