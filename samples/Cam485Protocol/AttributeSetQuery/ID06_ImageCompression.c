#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "elog.h"
#include "AttributeTable.h"
#include "cm_config.h"
int attribute_compression_ratio_set(const uint8_t* value, uint32_t value_len)
{
    log_i("attribute_compression_ratio_set%d",value_len);
    if (value == NULL || value_len != 1) 
    {
        log_e("Invalid parameter for attribute_compression_ratio_set");
        return -1;
    }
    uint8_t result = 0;
    log_i("Set Gyroscope enable status: %d", value[0]);
    int ret = Set_g_compressibility(value[0]);
    return SendSetAttributeResp(0x08, result);
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
    return SendGetAttributeResp(0x08, result, GetValue,valueLen);
}