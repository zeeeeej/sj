#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "elog.h"
#include "AttributeTable.h"
#include "cm_config.h"
#include "cm_video_ctrl.h"
static int Set_Video_Camera_Resolution(uint16_t width, uint16_t height)
{
    CMVideoCommand cmd_picture_size;
    cmd_picture_size.cmd_type = CMD_SINGLE_PARAM;
    cmd_picture_size.single_param.param_id = PARAM_RESOLUTION;
    cmd_picture_size.single_param.resolution.width = width;
    cmd_picture_size.single_param.resolution.height = height;
    return cm_video_ctrl(cmd_picture_size, NULL);
}
static int handle_picture_size(const uint8_t* value, uint32_t value_len) {
    uint16_t width = value[0] | (value[1] << 8);
    uint16_t height = value[2] | (value[3] << 8);

/*todo : 设置成功但是操作失败？*/
    int ret = Set_Camera_config(width, height);
    if(ret < 0)
    {
        return -1;
    }
    ret = Set_Video_Camera_Resolution(width, height);
    if(ret < 0)
    {
        return -1;
    }
    return 0;
}

int attribute_picture_size_set(const uint8_t* value, uint32_t value_len)
{
    if (value_len != 4) 
    {
        log_i("Invalid picture size value length %d", value_len);
        return -1;
    }
    uint8_t result = 0;
    int ret = handle_picture_size(value, value_len);
    if (ret < 0) 
    {
        log_i("Failed to set picture size");
        result = 1;
    }
    return SendSetAttributeResp(0x05, result);
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