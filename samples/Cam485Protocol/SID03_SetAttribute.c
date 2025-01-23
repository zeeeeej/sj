/*主动拍照*/
#include "cm_common.h"

#include "stdbool.h"
#include "SID03_SetAttribute.h"
#include "Cam485ProtocolCommon.h"
#include "ImageInfoList.h"
    
//AA 5A 01 03 02 00 00 00 0F 1E 97 5D
int SID03_SetAttribute(uint8_t *msg_buf, uint32_t msg_dlc)
{
    if (msg_buf == NULL) 
    {
        log_e("error:msg_len = %d",msg_dlc);
        return -1;
    }
     if (msg_buf[0] != 0xAA || msg_buf[1] != 0x5A) 
     {
        log_e("Error: Invalid frame header\n");
        return -1;
    }
    uint8_t command    = msg_buf[2];      
    uint8_t subcommand = msg_buf[3];     
    uint32_t len = (msg_buf[4]) | (msg_buf[5] << 8) | (msg_buf[6] << 16) | (msg_buf[7] << 24);
    printf("len:%d\n",len);
     
    if (len != msg_dlc - 10)  //// 9字节 = 帧头(2) + 命令(2) + 长度(4) + 校验码(2)
    {
        log_e("Error: Data length mismatch\n");
        return -1;
    }
    uint8_t id     = msg_buf[8];           // 属性 ID
    uint8_t *value = &msg_buf[9];          // 属性值起始地址
    log_i("Command: 0x%02X, Subcommand: 0x%02X\n", command, subcommand);
    log_i("Attribute ID: 0x%02X\n", id);
    // for (int i = 0; i < len - 1; i++) 
    // {
    //     log_i("%d", value[i]);
    // }
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
        Handle_SID03_Attribute_SloveAddress(value[0]);
        break;
    case SID03_Attribute_FirmwareVersion:
        Handle_SID03_Attribute_FirmwareVersion(value[0]);
        break;
    case SID03_Attribute_CameraNum:
        Handle_SID03_Attribute_CameraNum(value[0] | (value[1] << 8));
        break;
    case SID03_Attribute_Baudrate:
        Handle_SID03_Attribute_Baudrate(value[0] | (value[1] << 8));
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

int Handle_SID03_Attribute_SloveAddress(uint16_t slave_address)
{
    Set_g_slave_address(slave_address);
    buildResponse(SID03_Attribute_SloveAddress,0);
    return 0;
}

int Handle_SID03_Attribute_FirmwareVersion(uint16_t firmware_version)
{
     FILE *fp = fopen(VERSION_FILE, "w");  // 使用"w"模式会清除原有内容
    if (fp == NULL) {
        log_e("Failed to open version file");
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

