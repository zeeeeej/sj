#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "elog.h"
#include "AttributeTable.h"
#include "cm_config.h"

int attribute_picture_brightness_set(const uint8_t* value, uint32_t value_len)
{
    if (value == NULL || value_len != 1) 
    {
        log_e("Invalid parameter for gyro enable set");
        return -1;
    }
    uint8_t result = 0;
    log_i("Set Gyroscope enable status: %d", value[0]);
    int ret = Set_g_Luminance(value[0]);
    return SendSetAttributeResp(0x07, result);
}
int attribute_picture_brightness_get()
{
    uint8_t result = 0;
    uint8_t luminance = 0;
    int ret = Get_Luminance(&luminance);
    if(ret < 0)
    {
        result = 1;
    }
    uint8_t GetValue[1];
    GetValue[0] = luminance;
    uint32_t valueLen = sizeof(GetValue);
    return SendGetAttributeResp(0x07, result, GetValue,valueLen);
}