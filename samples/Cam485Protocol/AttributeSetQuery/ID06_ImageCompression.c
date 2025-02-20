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
    uint8_t result = 0;
    result = Get_compressibility();
    uint32_t valueLen = sizeof(result);
    if(result < 0)
    {
        return SendGetAttributeResp(0x08, result, result,valueLen);
    }
    return SendGetAttributeResp(0x08, 0,&result,valueLen);
}