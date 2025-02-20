#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "elog.h"
#include "AttributeTable.h"
#include "cm_config.h"
#include "door_detect.h"
int attribute_gyro_direction_set(const uint8_t* value, uint32_t value_len)
{   
    if (value == NULL || value_len != 1) {
        log_e("Invalid parameter for gyro direction set");
        return -1;
    }
    uint8_t result = 0;
    log_i("Set Gyroscope capture direction");
    if (Set_Gyroscope_Capture_image_direction(value[0]) != 0) {
        result = 1;
    }
    else
    {
        /*设置陀螺仪抓图方向*/
        set_gyroscope_capture_direction(value[0]);
    }
    return SendSetAttributeResp(0x0B,result);
}

int attribute_gyro_direction_get()
{
    uint8_t result = 0;
    log_i("Get Gyroscope capture direction");
    uint8_t direction;
    if (Get_Gyroscope_Capture_image_direction(&direction) != 0) {
        result = 1;
    }
    uint8_t GetValue[1];
    GetValue[0] = direction;
    uint32_t valueLen = sizeof(GetValue);
    return SendGetAttributeResp(0x0B,result,GetValue,valueLen);
}