#ifndef SID0C_SENDFIRMWAREUPDATEPACKET
#define SID0C_SENDFIRMWAREUPDATEPACKET
#include <stdio.h>
#include <stdint.h>

#define SID0C_MSG_RESP_DATA_LEN      (1)
#define SID0C_MSG_RESP_TOTAL_LEN     (11)

int SID0C_SendFirmwareUpdatePacket(uint8_t *msg_buf, uint32_t msg_dlc);
#endif