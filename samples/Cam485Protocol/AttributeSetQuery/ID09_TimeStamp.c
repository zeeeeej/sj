#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "elog.h"
#include "AttributeTable.h"
#include "cm_config.h"
#include <time.h>

/*由于底层设置时间戳不成功，利用差值的方式来记录时间戳*/
typedef struct timeStampManage 
{
    uint32_t difference;    //和系统时间戳的差值
    char     signedFlag;    //标志位 0：正偏 真正想要的时间戳 = 系统时间戳 + 差值 ，1:则 真正想要的时间戳 = 系统时间戳 - 差值
}timeStampManage;

static timeStampManage mtimeStampManage;

int Set_g_timeStamp(uint32_t timestamp)
{
    uint32_t t = (uint32_t)time(NULL);
    if(timestamp > t)
    {
        mtimeStampManage.difference = timestamp - t;
        mtimeStampManage.signedFlag = 0;
    }
    else
    {
        mtimeStampManage.difference = t - timestamp;
        mtimeStampManage.signedFlag = 1;
    }
    
    log_i("t=%lu\n", t);
    log_i("difference=%lu\n", mtimeStampManage.difference);
    return 0;
}

int Get_g_timeStamp(uint32_t *timestamp)
{
    uint32_t t = (uint32_t)time(NULL);
    log_i("t=%ld\n", t);
    log_i("difference=%lu\n", mtimeStampManage.difference);
    *timestamp = mtimeStampManage.signedFlag ? (t-mtimeStampManage.difference) : (t + mtimeStampManage.difference);
    return 0;
}

int showDate(uint32_t _timestamp) 
{
    // 假设时间戳为1609459200（对应2021 - 01 - 01 00:00:00）
    time_t timestamp = (time_t)_timestamp;
    struct tm *tm_info;
    char buffer[80];

    // 将时间戳转换为struct tm结构体
    tm_info = localtime(&timestamp);

    // 将struct tm结构体格式化为字符串
    strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", tm_info);

    log_i("转换后的时间: %s\n", buffer);

    return 0;
}

int attribute_timestamp_set(const uint8_t* value, uint32_t value_len)
{
    log_i("attribute_timestamp_set\n");
    uint32_t msg_dlc = value_len;
    if (msg_dlc < 4)
        return -1;

    uint8_t result = 0;
    uint32_t timestamp  = ((value[3] << 24) | (value[2] << 16) | (value[1] << 8) | value[0]);

    int ret = Set_g_timeStamp(timestamp);
    if (ret < 0) {
        log_e("Failed to set timestamp");
        result = 1;
    }
    log_i("当前时间戳timestamp: %u", timestamp);

    showDate(timestamp);

    SendSetAttributeResp(0x09,result);
    return 0;
}

int attribute_timestamp_get()
{
    log_i("attribute_timestamp_get\n");
    int ret = 0;
    uint8_t result = 0;
    uint32_t timestamp;
    ret = Get_g_timeStamp(&timestamp);
    if (ret < 0) {
        log_e("Failed to get timestamp");
        result = 1;
    }
    log_i("当前时间戳timestamp: %u", timestamp);

    showDate(timestamp);

    uint32_t ValueLen = sizeof(uint32_t);
    uint8_t  value[4];
    value[0] = 0xff & (timestamp >> 0);
    value[1] = 0xff & (timestamp >> 8);
    value[2] = 0xff & (timestamp >> 16);
    value[3] = 0xff & (timestamp >> 24);
    SendGetAttributeResp(0x09,result,value,ValueLen);
    return 0;
}

