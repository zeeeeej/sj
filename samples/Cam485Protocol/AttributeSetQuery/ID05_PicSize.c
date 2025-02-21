#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "elog.h"
#include "AttributeTable.h"
#include "cm_config.h"

static int handle_picture_size(const uint8_t* value, uint32_t value_len) {
    uint16_t width = value[0] | (value[1] << 8);
    uint16_t height = value[2] | (value[3] << 8);
    int ret = wind_sample_set_resolution(width, height);
    Set_Camera_config(width, height);
    return ret;
}

int attribute_picture_size_set(const uint8_t* value, uint32_t value_len)
{
    if (value_len != 4) 
    {
        log_i("Invalid picture size value length %d", value_len);
        return -1;
    }
    int ret = handle_picture_size(value, value_len);
    uint16_t valueLen = sizeof(ret);
    if (ret < 0) 
    {
        log_i("Failed to set picture size");
        return SendGetAttributeResp(0x05, ret, ret,valueLen);
    }
    return SendGetAttributeResp(0x05, 0, &ret,valueLen);
}
int attribute_picture_size_get()
{
    uint8_t result = 0;
    uint16_t width = 0;
    uint16_t height = 0;
    int ret = Get_Camera_config(&width, &height);
    if (ret < 0) {
        log_e("Failed to get camera configuration");
        result = 1;
    }
    uint8_t GetValue[4] = {width & 0xFF, (width >> 8) & 0xFF, height & 0xFF, (height >> 8) & 0xFF};
    uint32_t valueLen = sizeof(GetValue);
    return SendGetAttributeResp(0x05, result, GetValue, valueLen);
}