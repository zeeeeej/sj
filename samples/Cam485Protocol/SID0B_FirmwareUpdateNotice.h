#ifndef SID0B_FIRMWAREUPDATENOTICE  
#define SID0B_FIRMWAREUPDATENOTICE
#include <stdio.h>
#include <stdint.h>

#define SID0B_MSG_REQ_TOTAL_LEN      (30) 
#define SID0B_MSG_RESP_DATA_LEN      (5)
#define SID0B_MSG_RESP_TOTAL_LEN     (10+SID0B_MSG_RESP_DATA_LEN)

/*
* updadeable:可升级标志 0：可升级 其他值：不可升级
* offset：升级包偏移量，接收到的升级包固件大小
*/
typedef struct UpdatePacket{
    char updadeable;
    uint32_t offset;
}UpdatePacketStruct;


int SID0B_FirmwareUpdateNotice(uint8_t *msg_buf, uint32_t msg_dlc);
#endif