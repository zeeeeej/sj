/*主动拍照*/
#include "cm_common.h"

#include "stdbool.h"
#include "SID03_SetAttribute.h"
#include "ProtocolPort.h"
#include "ImageInfoList.h"
    

int SID03_SetAttribute(uint8_t *msg_buf, uint32_t msg_dlc)
{
    LOGD("SID03_SetAttribute\n");
}

int Handle_SID03_Attribute_SloveAddress(uint16_t slave_address)
{
    return 0;
}

int Handle_SID03_Attribute_FirmwareVersion(uint16_t firmware_version)
{
     FILE *fp = fopen(VERSION_FILE, "w");  // 使用"w"模式会清除原有内容
    if (fp == NULL) {
        printf("Failed to open version file");
        return -1;
    }
    fprintf(fp, "Version: %s\n", firmware_version);
    fclose(fp);
    buildResponse(SID03_Attribute_FirmwareVersion,0);
    return 0;
}

int Handle_SID03_Attribute_CameraNum(uint16_t camera_num)
{
    
    return 0;
}

int Handle_SID03_Attribute_Baudrate(uint16_t baudrate)
{
    return 0;
}

int Handle_SID03_Attribute_PictureSize(uint16_t picture_size_x, uint16_t picture_size_y)
{
    return 0;
}

int Handle_SID03_Attribute_Compressibility(uint16_t compressibility)
{
return 0;
}

int Handle_SID03_Attribute_PictureLuminance(uint16_t picture_luminance)
{
    int ret = wind_sample_set_luminance(picture_luminance);
    if(ret != 0)
    {
        buildResponse(SID03_Attribute_PictureLuminance,1);
        return -1;
    }
    system("pkill -f pwm_start.sh");
    system("sh /system/init/pwm_start.sh &");

    buildResponse(SID03_Attribute_PictureLuminance,0);
    return 0;
}

int Handle_SID03_Attribute_PictureArguments(uint16_t picture_arguments)
{
return 0;
}

int Handle_SID03_Attribute_TimeStamp()
{
    return 0;
}

int Handle_SID03_Attribute_GyroscopeAngle(uint16_t angle)
{
    return 0;
}

int Handle_SID03_Attribute_GyroscopeDirection(uint16_t direction)
{
    return 0;
}

int Handle_SID03_Attribute_GyroscopeConfig()
{
    return 0;
}

int Handle_SID03_Attribute_GyroscopeStorePicture(uint16_t store_picture)
{
    return 0;
}

int Handle_SID03_Attribute_HeatingWire(uint16_t heating_wire_rate)
{
    printf("heating_wire_rate : %d\n",heating_wire_rate);
    static char buffer[16];  
    snprintf(buffer, sizeof(buffer), "%u", heating_wire_rate);  
    save_to_config("PWM_Config", "pwm_duty_cycle",buffer);
    buildResponse(SID03_Attribute_HeatingWire,0);
    return 0;
}

void buildResponse(enum SID03_Attribute item,int response_code)
{
    char response[] = {};
    int index = 0;
    response[index++] = 0xAA;
    response[index++] = 0x5A;
    response[index++] = 0x01;
    response[index++] = 0x03;
    response[index++] = 0x02;
    response[index++] = item;
    response[index++] = response_code;
    response[index++] = 0x00;
    response[index++] = 0x00;

    send_msg_resp(response, index + 2);
}

