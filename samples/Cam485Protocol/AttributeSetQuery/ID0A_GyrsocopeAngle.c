#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "elog.h"
#include "MsgDispatcher.h"
#include "Cam485ProtocolCommon.h"
#include "cm_config.h"
#include "AttributeTable.h"
#include "door_detect.h"
static uint8_t gyroscope_capture_angle[2] = {0};


int attribute_gyro_angle_set(const uint8_t* value, uint32_t value_len)
{
    uint8_t result = 0;
    log_i("Set Gyroscope capture angle");
    gyroscope_capture_angle[0] = value[0];
    gyroscope_capture_angle[1] = value[1];
    if (Set_Gyroscope_Capture_image_angle(gyroscope_capture_angle[0], gyroscope_capture_angle[1]) != 0) {
        result = 1;
    }
    else
    {
        log_i("Set Gyroscope capture angle : %d, %d", value[0], value[1]); 

        if(gyroscope_capture_angle[0]!=0)
        {
            /*修改拍照角度*/
            set_gyroscope_capture_angle(gyroscope_capture_angle[0]);
        }
        else if(gyroscope_capture_angle[1]!=0)
        {

            /*修改拍照角度*/
            set_gyroscope_capture_angle(gyroscope_capture_angle[1]);
        }
        else
        {
            /*两个为0 不做任何修改*/
        }

    }


    return SendSetAttributeResp(0x0A,result);
}


int attribute_gyro_angle_get()
{
    uint8_t angleA = 0;
    uint8_t angleB = 0;
    uint8_t result = 0;
    if(Get_Gyroscope_Capture_image_angle(&angleA, &angleB) != 0)
    {
        result = 1;
    }
    uint8_t GetValue[2];
    GetValue[0] = angleA;
    GetValue[1] = angleB;
    uint32_t valueLen = sizeof(GetValue);
    return SendGetAttributeResp(0x0A, result, GetValue, valueLen);
}