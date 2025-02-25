#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "elog.h"
#include "AttributeTable.h"
#include "cm_config.h"
#include "cm_video_ctrl.h"
static int Set_Video_Compressibility(uint8_t value)
{
    CMVideoCommand cmd_compressibility;
    cmd_compressibility.cmd_type = CMD_SINGLE_PARAM;
    cmd_compressibility.single_param.param_id = PARAM_COMPRESSION;
    cmd_compressibility.single_param.value = value; // 设置亮度值为128
    return cm_video_ctrl(cmd_compressibility, NULL);
}


int attribute_compression_ratio_set(const uint8_t* value, uint32_t value_len)
{
    
    if (value == NULL || value_len != 1) 
    {
        log_e("Invalid parameter for attribute_compression_ratio_set");
        return -1;
    }
    log_i("set compression ratio");
    uint8_t result = 0;
    int ret = Set_g_compressibility(value[0]);
    if(ret < 0)
    {
        result = 1; 
    }
    else
    {
        ret = Set_Video_Compressibility(value[0]);
        if(ret < 0)
        {
            result = 1;
        }
    }
    return SendSetAttributeResp(0x06, result);
}
int attribute_compression_ratio_get()
{
    int ret = 0;
    uint8_t result = 0;
    uint8_t compression = 0;
    ret = Get_compressibility(&compression);
    if(ret < 0)
    {
        result = 1; 
    }
    uint8_t GetValue[1];
    GetValue[0] = compression;
    uint32_t valueLen = sizeof(GetValue);
    return SendGetAttributeResp(0x06, result, GetValue,valueLen);
}