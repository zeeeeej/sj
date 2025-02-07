#include <stdio.h>
#include "malloc.h"
#include <string.h>  // 包含 strerror 声明
#include <errno.h>   // 包含 errno 定义
#include <limits.h>  // 包含 PATH_MAX 定义
/*固件升级通知*/
#include "cm_common.h"

#include "stdbool.h"
#include "SID0B_FirmwareUpdateNotice.h"
#include "Cam485ProtocolCommon.h"
#include "ImageInfoList.h"

static UpdatePacketStruct  UpdatePacketInfo;

void SID0BC_GetFirmwaveUpdateInfo(UpdatePacketStruct *pUpdatePacketInfo)
{
    pUpdatePacketInfo->updadeable   = UpdatePacketInfo.updadeable;
    pUpdatePacketInfo->len          = UpdatePacketInfo.len;
    strncpy(pUpdatePacketInfo->md5_str,UpdatePacketInfo.md5_str,sizeof(UpdatePacketInfo.md5_str));
    pUpdatePacketInfo->offset       = UpdatePacketInfo.offset;
}

void SID0BC_SetFirmwaveUpdateInfo(UpdatePacketStruct *pUpdatePacketInfo)
{
    UpdatePacketInfo.offset += pUpdatePacketInfo->offset;
}

void SID0B_InitFirmwaveUpdateInfo(UpdatePacketStruct *pUpdatePacketInfo,const uint8_t *msg_buf, uint32_t msg_dlc)
{
    pUpdatePacketInfo->offset = 0;

    uint32_t filesize = ((msg_buf[11] << 24) | (msg_buf[10] << 16) | (msg_buf[9] << 8) | msg_buf[8]);
    pUpdatePacketInfo->len = filesize;

    char md5_str[33] = {0};
    // Convert to hex string
    for (int i = 0; i < 16; i++) {
        sprintf(&md5_str[i * 2], "%02x", msg_buf[i+12]);
    }
    md5_str[32] = '\0';
    strcpy(pUpdatePacketInfo->md5_str,md5_str);

    pUpdatePacketInfo->updadeable = 0;
}

int SID0B_FirmwareUpdateNoticeCheck(uint8_t *msg_buf, uint32_t msg_dlc)
{
    int ret = 0;

    (void)msg_buf;
    if (msg_dlc < SID0B_MSG_REQ_TOTAL_LEN)
        return -1;

    uint16_t crc_msg = crc16(msg_buf, msg_dlc - 2);
    printf("cacul crc_msg : %04X\n",crc_msg);
    if(((crc_msg & 0xFF) != msg_buf[msg_dlc - 1]) || (msg_buf[msg_dlc - 1] != crc_msg >> 8))
    {
        LOGD("ERROR:CRC16 ERROR \n");
        LOGD("%02X %02X",msg_buf[msg_dlc - 2],msg_buf[msg_dlc - 1]);
        //return -1;
    }
    
    return ret;
}

static int SID0B_BuildMsgHeader(uint8_t *msg_buf, uint32_t msg_dlc)
{
    int index = 0;
    msg_buf[index++] = 0xAA;
    msg_buf[index++] = 0x5A;
    msg_buf[index++] = SLAVE_ADDR;
    msg_buf[index++] = 0x0B;

    /*填充长度字段*/
    msg_buf[index++] = (uint8_t)(msg_dlc & 0xFF);         // Low byte
    msg_buf[index++] = (uint8_t)((msg_dlc >> 8) & 0xFF);  // 2nd byte
    msg_buf[index++] = (uint8_t)((msg_dlc >> 16) & 0xFF); // 3rd byte
    msg_buf[index++] = (uint8_t)((msg_dlc >> 24) & 0xFF); // High byte
}   

int SID0B_FirmwareUpdateNotice(uint8_t *msg_buf, uint32_t msg_dlc)
{
    LOGD("SID0B_FirmwareUpdateNotice\n");
    LOGD("SID0B_FirmwareUpdateNotice %s%s\n",__DATE__,__TIME__);
    uint8_t resp_buf[SID0B_MSG_RESP_TOTAL_LEN];
    memset(resp_buf, 0, SID0B_MSG_RESP_TOTAL_LEN);
    int ret = 0;
    uint8_t result = 0;
    uint8_t id;
    ret = SID0B_FirmwareUpdateNoticeCheck(msg_buf, msg_dlc);
    if (ret != 0)
    {
        LOGD("error:SID0B_FirmwareUpdateNoticeCheck\n");
        return -1;
    }
    
    SID0B_BuildMsgHeader(resp_buf, SID0B_MSG_RESP_DATA_LEN);

    SID0B_InitFirmwaveUpdateInfo(&UpdatePacketInfo,msg_buf, msg_dlc);

    FILE *fp = fopen(OTA_FILE_INFO_PATH, "r");
    if (!fp) 
    {
        LOGD("Failed to open %s file: %s, ret[-5]\n", OTA_FILE_INFO_PATH,strerror(errno));
        //return -2;
    }

    char ota_file_info[128] = {0};

    sprintf(ota_file_info,"{MD5:%s}{SIZE:%u}{UPDATE:%d}",UpdatePacketInfo.md5_str,UpdatePacketInfo.len,UpdatePacketInfo.updadeable);
    printf("%s\n",ota_file_info);

    char ota_file_info_read[128] = {0}; 
    /*获取本地升级文件信息*/
    uint16_t bytes_read = fread(ota_file_info_read, 1, sizeof(ota_file_info_read), fp);
    LOGD("bytes_read: %d", bytes_read);
    LOGD("new: %s", ota_file_info);
    LOGD("old: %s", ota_file_info_read);
    fclose(fp);

    char md5_str[33] = {0};
    memcpy(md5_str,ota_file_info_read+sizeof("{MD5:")-1,32);
    md5_str[32] = '\0';

    LOGD("md5_str: %s", md5_str);
    LOGD("UpdatePacketInfo.md5_str: %s", UpdatePacketInfo.md5_str);

    system("rm /system/sample_camera_rst.xz");
    system("touch /system/sample_camera_rst.xz");
    // 比较MD5值
    if (strcmp(md5_str, UpdatePacketInfo.md5_str) == 0)
    {
        LOGD("not new bin file");
        
        resp_buf[8]     = 1;
        resp_buf[9]     = 0;
        resp_buf[10]    = 0;
        resp_buf[11]    = 0;
        resp_buf[12]    = 0;
        send_msg_resp(resp_buf, SID0B_MSG_RESP_TOTAL_LEN);
        return 0;
    }
    LOGD("is new bin file");

    resp_buf[8] = 1;
    resp_buf[9] = 0;
    resp_buf[10] = 0;
    resp_buf[11] = 0;
    resp_buf[12] = 0;

    send_msg_resp(resp_buf, SID0B_MSG_RESP_TOTAL_LEN);
    return 0;
}
