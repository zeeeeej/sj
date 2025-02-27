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

#define FILE_DIR "/tmp/"
#define OTA_FILE_INFO_PATH "/system/md5.xml"
#define OTA_FILE_PATH   FILE_DIR "sample_camera_rst.xz"
#define RM_OTA_FILE_PATH  "rm " OTA_FILE_PATH
#define TOUCH_OTA_FILE_PATH  "touch " OTA_FILE_PATH

#define OTA_SHELL_PATH "/tmp/myotatest.sh"
#define CAT_OTA_SHELL_PATH "cat " OTA_SHELL_PATH
#define CHMOD_OTA_SHELL_PATH "chmod 777 " OTA_SHELL_PATH
#define SH_OTA_SHELL_PATH "sh " OTA_SHELL_PATH
#define RM_OTA_SHELL_PATH "rm " OTA_SHELL_PATH

void SID0BC_GetFirmwaveUpdateInfo(UpdatePacketStruct *pUpdatePacketInfo);
#endif