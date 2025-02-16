#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "elog.h"
#include "AttributeTable.h"
#include "cm_config.h"
#include "door_detect.h"
static int Enable_Gyroscope()
{
    return door_detect_init();
}

static int Disable_Gyroscope()
{
    return door_detect_deinit();
}







/*属性设置处理函数*/
int attribute_gyro_enable_set(const uint8_t* value, uint32_t value_len)
{
    if (value == NULL || value_len != 1) {
        log_e("Invalid parameter for gyro enable set");
        return -1;
    }
    uint8_t result = 0;
    // 验证使能状态值是否有效（0或1）
    if (value[0] != 0 && value[0] != 1) {
        log_e("Invalid enable status value: %d", value[0]);
        return -1;
    }

    log_i("Set Gyroscope enable status: %d", value[0]);
    if(Set_Gyroscope_Enable_Status(value[0])!=0)
    {
        result = 1;
    }
    else{
        if(value[0] == 1)
        {
            log_i("Gyroscope set enable");
            Enable_Gyroscope();
        }
        else
        {
            log_i("Gyroscope set disable");
            Disable_Gyroscope();
        }
    }
    return SendSetAttributeResp(0x0D,result);
}

/*属性获取处理函数*/
int attribute_gyro_enable_get(uint8_t* value, uint32_t* value_len)
{
    if (value == NULL || value_len == NULL || *value_len < 1) {
        log_e("Invalid parameter for gyro enable get");
        return -1;
    }
    uint8_t result = 0;
    uint8_t enable_status;
    if (Get_Gyroscope_Enable_Status(&enable_status) != 0) {
        result = 1;
    }

    uint8_t Getvalue[1];
    Getvalue[0]  = enable_status;
    uint32_t ValueLen = sizeof(Getvalue);
    return SendGetAttributeResp(0x0D,result,Getvalue,ValueLen);
}