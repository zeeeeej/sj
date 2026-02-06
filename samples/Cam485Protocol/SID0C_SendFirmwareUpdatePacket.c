#include <stdio.h>
#include "malloc.h"
#include <string.h>  // 包含 strerror 声明
#include <errno.h>   // 包含 errno 定义
#include <limits.h>  // 包含 PATH_MAX 定义

/*获取升级文件*/
#include "cm_common.h"
#include "stdbool.h"
#include "SID0BC_FirmwareUpdateInfo.h"
#include "SID0C_SendFirmwareUpdatePacket.h"
#include "Cam485ProtocolCommon.h"
#include "ImageInfoList.h"
#include "md5.h"
#include "hd_ota.h"

int SID0C_GetFirmwaveUpdateFlag(void)
{
    UpdatePacketStruct UpdatePacketInfo;
    SID0BC_GetFirmwaveUpdateInfo(&UpdatePacketInfo);
    if(0==UpdatePacketInfo.updadeable)
    {
        return 0;
    }
    return -1;
}

int SID0C_GetFirmwaveMD5(char *md5Str)
{
    UpdatePacketStruct UpdatePacketInfo;
    SID0BC_GetFirmwaveUpdateInfo(&UpdatePacketInfo);
    UpdatePacketInfo.md5_str[32] = '\0';
    strcpy(md5Str,UpdatePacketInfo.md5_str);
    return 0;
}

int SID0C_GetFirmwaveSize(void)
{
    UpdatePacketStruct UpdatePacketInfo;
    SID0BC_GetFirmwaveUpdateInfo(&UpdatePacketInfo);
    return  UpdatePacketInfo.len;
}

int SID0C_SendFirmwareUpdatePacketCheck(const uint8_t *msg_buf, uint32_t msg_dlc)
{
    int ret = 0;

    (void)msg_buf;
    // if (msg_dlc < SID0C_MSG_REQ_TOTAL_LEN)  
    //     ret = -1;
    if (msg_dlc < ((msg_buf[4]) | (msg_buf[5] << 8) | (msg_buf[6] << 16) | (msg_buf[7] << 24) ))
        ret = -1;

    uint16_t crc_msg = crc16(msg_buf, msg_dlc - 2);
    printf("cacul crc_msg : %04X\n",crc_msg);
    printf("cacul crc_msg   : %02X %02X\n",crc_msg & 0xFF,crc_msg >> 8);
    LOGD("msg_dlc:%d",msg_dlc);
    if(((crc_msg & 0xFF) != msg_buf[msg_dlc - 2]) || (msg_buf[msg_dlc - 1] != crc_msg >> 8))
    {
        LOGD("ERROR:CRC16 ERROR\n");
        LOGD("CRC16:%02X %02X",msg_buf[msg_dlc - 2],msg_buf[msg_dlc - 1]);
        return -1;
    }

    return ret;
}

static int SID0C_BuildMsgHeader(uint8_t *msg_buf, uint32_t msg_dlc)
{
    uint8_t slave_address;
    Get_Protocol_Slave_Address(&slave_address);
    int index = 0;
    msg_buf[index++] = 0xAA;
    msg_buf[index++] = 0x5A;
    msg_buf[index++] = slave_address;
    msg_buf[index++] = 0x0C;

    /*填充长度字段*/
    msg_buf[index++] = (uint8_t)(msg_dlc & 0xFF);         // Low byte
    msg_buf[index++] = (uint8_t)((msg_dlc >> 8) & 0xFF);  // 2nd byte
    msg_buf[index++] = (uint8_t)((msg_dlc >> 16) & 0xFF); // 3rd byte
    msg_buf[index++] = (uint8_t)((msg_dlc >> 24) & 0xFF); // High byte
}


int makeotashell()
{
    // 定义一维字符数组存储多行字符串
    char str[] = OTASHELLDATA;
    FILE *fp = fopen(OTA_SHELL_PATH, "w");
    if (fp == NULL) {
        perror("fopen error");
        return 1;
    }
    // 将字符数组中的内容写入文件
    fputs(str, fp);
    fclose(fp);
    printf("fopen success\n");
    return 0;
}

int SID0C_SendFirmwareUpdatePacket(uint8_t *msg_buf, uint32_t msg_dlc)
{
    LOGD("SID0C_SendFirmwareUpdatePacketshot");
    uint8_t resp_buf[SID0C_MSG_RESP_TOTAL_LEN];
    memset(resp_buf, 0, SID0C_MSG_RESP_TOTAL_LEN);
    int ret = 0;
    uint8_t result = 0;
    uint8_t id;
    ret = SID0C_SendFirmwareUpdatePacketCheck(msg_buf, msg_dlc);
    if (ret != 0)
    {
        LOGD("resend update firmwave \n");
        SID0C_BuildMsgHeader(resp_buf, SID0C_MSG_RESP_DATA_LEN);
        resp_buf[8] = 2;
        send_msg_resp(resp_buf, SID0C_MSG_RESP_TOTAL_LEN);
        return -1;
    }

    /*判断是否可以升级*/
    if(0!=SID0C_GetFirmwaveUpdateFlag())
    {
        LOGD("can not update firmwave  \n");
        SID0C_BuildMsgHeader(resp_buf, SID0C_MSG_RESP_DATA_LEN);
        resp_buf[8] = 1;
        send_msg_resp(resp_buf, SID0C_MSG_RESP_TOTAL_LEN);
        return -2;
    }

    FILE * fp = fopen(OTA_FILE_PATH, "r+");
    if (!fp) 
    {
        LOGD("Failed to open %s file: %s,ret[-5]\n", OTA_FILE_PATH,strerror(errno));
        return -3;
    }


    uint32_t write_len  = ((msg_buf[15] << 24) | (msg_buf[14] << 16) | (msg_buf[13] << 8) | msg_buf[12]);
    uint32_t offset     = ((msg_buf[11] << 24) | (msg_buf[10] << 16) | (msg_buf[9] << 8) | msg_buf[8]);
    
    LOGD("SID0C  write_len  = %d",write_len);
    static int sum = 0;
    sum += write_len;
    LOGD("SID0C sum  = %d",offset);
    
    /* 定位文件指针到指定偏移 */
    if (fseek(fp, offset, SEEK_SET) != 0) 
    {
        LOGD("SID0C Failed to seek to offset %lu in file: %s, ret[-6]\n", offset, strerror(errno));
        fclose(fp);
        return -2;
    }
    
    fwrite(&(msg_buf[16]),sizeof(uint8_t), write_len, fp);

    fclose(fp);
    
    if((offset+write_len) > SID0C_GetFirmwaveSize())
    {
        LOGD("error:firmwaveOffset more then firmwaveSizxe.");
        return -4;
    }

    else if((offset+write_len) == SID0C_GetFirmwaveSize())
    {
        unsigned char digest[16]={0};

        // 计算文件MD5
        if (calculate_file_md5(OTA_FILE_PATH, digest) != 0)
        {
            LOGD("Failed to calculate file MD5");
            return -6;
        }

        char calculated_md5[33] = {0};
        for (int i = 0; i < 16; i++) {
            sprintf(&calculated_md5[i * 2], "%02x", digest[i]);
        }
        calculated_md5[32] = '\0';

        char md5_str[33] = {0};
        SID0C_GetFirmwaveMD5(md5_str);

        LOGD("Expected: %s", md5_str);
        LOGD("Calculated: %s", calculated_md5);
        // 比较MD5值
        if (strcmp(calculated_md5, md5_str) == 0)
        {
            SID0C_BuildMsgHeader(resp_buf, SID0C_MSG_RESP_DATA_LEN);
            resp_buf[8] = 0;
            //printf("After header: msg_buf[8] = 0x%02X, msg_buf[9] = 0x%02X\n", resp_buf[8], resp_buf[9]);
            send_msg_resp(resp_buf, SID0C_MSG_RESP_TOTAL_LEN);

            FILE *fp = fopen(OTA_FILE_INFO_PATH, "w+");
            if (!fp)
            {
                LOGD("Failed to open %s file: %s, ret[-6]\n", OTA_FILE_INFO_PATH,strerror(errno));
                //return -2;
            }
            

            char ota_file_info[128] = {0};

            sprintf(ota_file_info,"{MD5:%s}{SIZE:%u}{UPDATE:%d}",md5_str,SID0C_GetFirmwaveSize(),SID0C_GetFirmwaveUpdateFlag());
            printf("SID0C %s\n",ota_file_info);

            if(sizeof(ota_file_info) != fwrite(ota_file_info,sizeof(uint8_t), sizeof(ota_file_info), fp))
            {
                LOGD("error:write update firmwave  info");
                //ret = -2;
            }

            fclose(fp);
 
            printf("updating.....\n");
            usleep(1000*100);

            //system("chmod 777 /system/init/myotatest.sh;sh /system/init/myotatest.sh");
            makeotashell();
            system(CAT_OTA_SHELL_PATH);
            system(CHMOD_OTA_SHELL_PATH);
            system(SH_OTA_SHELL_PATH);
        }
    }
    else
    {
        SID0C_BuildMsgHeader(resp_buf, SID0C_MSG_RESP_DATA_LEN);
        resp_buf[8] = 1;
        //printf("After header:  msg_buf[8] = 0x%02X,   aamsg_buf[9] = 0x%02X\n", resp_buf[8], resp_buf[9]);
        printf("request next pack\n");
        send_msg_resp(resp_buf, SID0C_MSG_RESP_TOTAL_LEN);
    }

     return 0;

}

int hd_ota_poll_file(){
    LOGD("--> hd_ota_poll_file");
    uint8_t resp_buf[SID0C_MSG_RESP_TOTAL_LEN];
    memset(resp_buf, 0, SID0C_MSG_RESP_TOTAL_LEN);
    int ret = 0;
    uint8_t result = 0;
    uint8_t id;
  
    /*判断是否可以升级*/
    if(0!=SID0C_GetFirmwaveUpdateFlag())
    {
        LOGD("can not update firmwave  \n");
        SID0C_BuildMsgHeader(resp_buf, SID0C_MSG_RESP_DATA_LEN);
        resp_buf[8] = 1;
        send_msg_resp(resp_buf, SID0C_MSG_RESP_TOTAL_LEN);
        return -2;
    }

    FILE * fp = fopen(OTA_FILE_PATH, "r+");
    if (!fp) 
    {
        LOGD("Failed to open %s file: %s,ret[-5]\n", OTA_FILE_PATH,strerror(errno));
        return -3;
    }

    int file_size = SID0C_GetFirmwaveSize();
    if (file_size<=0)
    {
        return -2;
    }
    
    uint8_t  msg_buf [1024*1024*2];
    int read_size = hd_read_file(msg_buf,file_size);
    printf("--> hd_ota_poll_file read_size = %d\n",read_size);
    if (read_size!=file_size)
    {
        printf("--> hd_ota_poll_file read_size!=file_size %d != %d\n",read_size,file_size);
        return -1;
    }
    
    fwrite(msg_buf,sizeof(uint8_t), read_size, fp);

    unsigned char digest[16]={0};

    // 计算文件MD5
    if (calculate_file_md5(OTA_FILE_PATH, digest) != 0)
    {
        LOGD("Failed to calculate file MD5");
        return -6;
    }

    char calculated_md5[33] = {0};
    for (int i = 0; i < 16; i++) {
        sprintf(&calculated_md5[i * 2], "%02x", digest[i]);
    }
    calculated_md5[32] = '\0';

    char md5_str[33] = {0};
    SID0C_GetFirmwaveMD5(md5_str);

    LOGD("Expected: %s", md5_str);
    LOGD("Calculated: %s", calculated_md5);
    // 比较MD5值
    if (strcmp(calculated_md5, md5_str) == 0)
    {
        SID0C_BuildMsgHeader(resp_buf, SID0C_MSG_RESP_DATA_LEN);
        resp_buf[8] = 0;
        //printf("After header: msg_buf[8] = 0x%02X, msg_buf[9] = 0x%02X\n", resp_buf[8], resp_buf[9]);
        send_msg_resp(resp_buf, SID0C_MSG_RESP_TOTAL_LEN);

        FILE *fp = fopen(OTA_FILE_INFO_PATH, "w+");
        if (!fp)
        {
            LOGD("Failed to open %s file: %s, ret[-6]\n", OTA_FILE_INFO_PATH,strerror(errno));
            //return -2;
        }
        

        char ota_file_info[128] = {0};

        sprintf(ota_file_info,"{MD5:%s}{SIZE:%u}{UPDATE:%d}",md5_str,SID0C_GetFirmwaveSize(),SID0C_GetFirmwaveUpdateFlag());
        printf("SID0C %s\n",ota_file_info);

        if(sizeof(ota_file_info) != fwrite(ota_file_info,sizeof(uint8_t), sizeof(ota_file_info), fp))
        {
            LOGD("error:write update firmwave  info");
            //ret = -2;
        }

        fclose(fp);

        printf("updating.....\n");
        usleep(1000*100);

        //system("chmod 777 /system/init/myotatest.sh;sh /system/init/myotatest.sh");
        makeotashell();
        system(CAT_OTA_SHELL_PATH);
        system(CHMOD_OTA_SHELL_PATH);
        system(SH_OTA_SHELL_PATH);
        printf("--> prepare reboot !\n");
        sleep(1);
        system("reboot");
    }else{
         printf("--> hd_ota_poll_file file md5 error !\n");
         return -9;
    }

    return 0;    

}




