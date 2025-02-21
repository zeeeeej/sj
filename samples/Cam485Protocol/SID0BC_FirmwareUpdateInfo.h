#ifndef SID0B_FIRMWAREUPDATEINFO  
#define SID0B_FIRMWAREUPDATEINFO
#include <stdio.h>
#include <stdint.h>

/*
* updadeable:可升级标志 0：可升级 其他值：不可升级
* offset：升级包偏移量，接收到的升级包固件大小
*/
typedef struct UpdatePacket{
    uint32_t offset;
    uint32_t len;
    char md5_str[33];
    char updadeable;
}UpdatePacketStruct;

#define OTA_FILE_INFO_PATH "/system/md5.xml"
#define OTA_FILE_PATH   "/tmp/sample_camera_rst.xz"

void SID0BC_GetFirmwaveUpdateInfo(UpdatePacketStruct *pUpdatePacketInfo);
#endif