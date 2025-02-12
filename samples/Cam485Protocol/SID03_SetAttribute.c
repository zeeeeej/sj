/*主动拍照*/
#include "cm_common.h"

#include "stdbool.h"
#include "SID03_SetAttribute.h"
#include "Cam485ProtocolCommon.h"
#include "ImageInfoList.h"
#include "SID03_SetAttribute_ID1_4.h"
    
//AA 5A 01 03 02 00 00 00 0F 1E 97 5D
int SID03_SetAttribute(uint8_t *msg_buf, uint32_t msg_dlc)
{
    uint8_t id     = msg_buf[8];           // 属性 ID
    uint8_t *value = &msg_buf[9];          // 属性值起始地址

    uint16_t width,height;
    if (id == 5)
    {
        width  = value[0] | (value[1] << 8); // 小端解析宽度
        height = value[2] | (value[3] << 8); // 小端解析高度
        log_i("Width = %d, Height = %d\n", width, height);
    }
    switch (id) 
    {
    case SID03_Attribute_SloveAddress:
        Handle_SID03_Attribute_SloveAddress(msg_buf,msg_dlc);
        break;
    case SID03_Attribute_FirmwareVersion:
        Handle_SID03_Attribute_FirmwareVersion(msg_buf,msg_dlc);
        break;
    case SID03_Attribute_CameraNum:
        Handle_SID03_Attribute_CameraNum(msg_buf,msg_dlc);
        break;
    case SID03_Attribute_Baudrate:
        Handle_SID03_Attribute_Baudrate(msg_buf,msg_dlc);
        break;
    case SID03_Attribute_PictureSize:
        Handle_SID03_Attribute_PictureSize(width, height);
        break;
    case SID03_Attribute_Compressibility:
        Handle_SID03_Attribute_Compressibility(value[0] | (value[1] << 8));
        break;
    case SID03_Attribute_PictureLuminance:
        Handle_SID03_Attribute_PictureLuminance(value[0] | (value[1] << 8));
        break;
    case SID03_Attribute_PictureArguments:
        Handle_SID03_Attribute_PictureArguments(value[0] | (value[1] << 8));
        break;
    case SID03_Attribute_TimeStamp:
        Handle_SID03_Attribute_TimeStamp();
        break;
    case SID03_Attribute_GyroscopeAngle:
        Handle_SID03_Attribute_GyroscopeAngle(value[0] | (value[1] << 8));
        break;
    case SID03_Attribute_GyroscopeDirection:
        Handle_SID03_Attribute_GyroscopeDirection(value[0] | (value[1] << 8));
        break;
    case SID03_Attribute_GyroscopeConfig:
        Handle_SID03_Attribute_GyroscopeConfig();
        break;
    case SID03_Attribute_GyroscopeStorePicture:
        Handle_SID03_Attribute_GyroscopeStorePicture(value[0] | (value[1] << 8));
        break;
    case SID03_Attribute_HeatingWire:
        Handle_SID03_Attribute_HeatingWire(value[0]);
        break;
    default:
        log_e("Error: Unknown attribute ID\n");
        break;
    }
    return 0;
}

int Handle_SID03_Attribute_PictureSize(uint16_t picture_size_x, uint16_t picture_size_y)
{
    int ret = wind_sample_set_resolution(picture_size_x, picture_size_y);
    Set_Camera_config(picture_size_x, picture_size_y);
    buildResponse(SID03_Attribute_PictureSize,0);
    return 0;
}

int Handle_SID03_Attribute_Compressibility(uint16_t compressibility)
{
return 0;
}

int Handle_SID03_Attribute_PictureLuminance(uint16_t picture_luminance)
{
    Set_Luminance(picture_luminance);
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
    save_to_config("PWM_Config", "DUTY_CYCLE",buffer);
    save_to_config("PWM_Config", "GPIO_PIN", "49");
    save_to_config("PWM_Config", "FREQUENCY", "1000");
    write_ini();
    system("pkill -f pwm_start.sh");
    system("/system/init/pwm_start.sh &");
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
    response[index++] = 0x00;
    response[index++] = 0x00;
    response[index++] = 0x00;
    response[index++] = item;
    response[index++] = response_code;
   

    send_msg_resp(response, index + 2);
}

