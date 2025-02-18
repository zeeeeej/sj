#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "elog.h"

int attribute_gyro_angle_set(const uint8_t* value, uint32_t value_len)
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
    return SendSetAttributeResp(0x0A,result);
}


int attribute_gyro_angle_get(uint8_t* value, uint32_t* value_len)
{
    return 0;
}