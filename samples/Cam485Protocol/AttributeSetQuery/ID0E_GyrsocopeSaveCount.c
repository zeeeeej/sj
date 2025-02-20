#define LOG_TAG "[ID0E_GyrsocopeSaveCount]"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "elog.h"
#include "AttributeTable.h" 
#include "MsgDispatcher.h"
#include "Cam485ProtocolCommon.h"
#include "cm_config.h"

int attribute_gyro_store_picture_set(const uint8_t* value, uint32_t value_len)
{
    if (value == NULL || value_len != 1) {
        log_e("Invalid parameter for gyro save count set");
        return -1;
    }
    uint8_t result = 0;
    // 验证使能状态值是否有效（0或1）
    if (value[0] != 0 && value[0] != 1) {
        log_e("Invalid save count value: %d", value[0]);
        return -1;
    }

    log_i("Set Gyroscope save count: %d", value[0]);
    if(Set_Gyroscope_Image_Save_Count(value[0])!=0)
    {
        result = 1;
    }
    return SendSetAttributeResp(0x0E,result);
}
int attribute_gyro_store_picture_get()
{

    uint8_t result = 0;
    uint8_t enable_status;
    if (Get_Gyroscope_Image_Save_Count(&enable_status) != 0) {
        result = 1;
    }

    uint8_t Getvalue[1];
    Getvalue[0]  = enable_status;
    uint32_t ValueLen = sizeof(Getvalue);
    return SendGetAttributeResp(0x0E,result,Getvalue,ValueLen);
}