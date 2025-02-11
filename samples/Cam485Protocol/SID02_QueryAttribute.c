
#include "cm_common.h"

#include "stdbool.h"
#include "SID02_QueryAttribute.h"
#include "Cam485ProtocolCommon.h"
#include "ImageInfoList.h"


int SID02_QueryAttribute(uint8_t *msg_buf, uint32_t msg_dlc)
{
    LOGD("SID02_QueryAttribute\n");
    if (msg_buf == NULL || msg_dlc != 12) 
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
    log_i("len:%d\n",len);
    if (len != msg_dlc - 10)  //// 10字节 = 帧头(2) + 命令(2) + 长度(4) + 校验码(2)
    {
        log_e("Error: Data length mismatch\n");
        return -1;
    }
    uint8_t id     = msg_buf[8];           // 属性 ID
    
    log_i("Command: 0x%02X, Subcommand: 0x%02X\n", command, subcommand);
    log_i("Attribute ID: 0x%02X\n", id);
    switch (id) 
    {
    case SID02_Attribute_SloveAddress:
        Handle_SID02_Attribute_SloveAddress();
        break;
    case SID02_Attribute_FirmwareVersion:
        Handle_SID02_Attribute_FirmwareVersion();
        break;
    case SID02_Attribute_CameraNum:
        Handle_SID02_Attribute_CameraNum();
        break;
    case SID02_Attribute_Baudrate:
        Handle_SID02_Attribute_Baudrate();
        break;
    case SID02_Attribute_PictureSize:
        Handle_SID02_Attribute_PictureSize();
        break;
    case SID02_Attribute_Compressibility:
        Handle_SID02_Attribute_Compressibility();
        break;
    case SID02_Attribute_PictureLuminance:
        Handle_SID02_Attribute_PictureLuminance();
        break;
    case SID02_Attribute_PictureArguments:
        Handle_SID02_Attribute_PictureArguments();
        break;
    case SID02_Attribute_TimeStamp:
        Handle_SID02_Attribute_TimeStamp();
        break;
    case SID02_Attribute_GyroscopeAngle:
        Handle_SID02_Attribute_GyroscopeAngle();
        break;
    case SID02_Attribute_GyroscopeDirection:
        Handle_SID02_Attribute_GyroscopeDirection();
        break;
    case SID02_Attribute_GyroscopeConfig:
        Handle_SID02_Attribute_GyroscopeConfig();
        break;
    case SID02_Attribute_GyroscopeOpenning:
        Handle_SID02_Attribute_GyroscopeOpenning();
        break;
    case SID02_Attribute_GyroscopeStorePicture:
        Handle_SID02_Attribute_GyroscopeStorePicture();
        break;
    case SID02_Attribute_HeatingWire:
        Handle_SID02_Attribute_HeatingWire();
        break;
    default:
        log_e("Error: Unknown attribute ID\n");
        break;
    }
}

int Handle_SID02_Attribute_SloveAddress()
{
    const char* slave_address = Get_g_slave_address();
    build_response(SID02_Attribute_SloveAddress, 0, 1, atoi(slave_address));
    return 0;
}

int Handle_SID02_Attribute_FirmwareVersion()
{
    const char* firmware_version = ReadFirware();
    build_response(SID02_Attribute_FirmwareVersion, 0, 1, atoi(firmware_version));
    return 0;
}

int Handle_SID02_Attribute_CameraNum()
{
    return 0;
}

int Handle_SID02_Attribute_Baudrate()
{
    return 0;
}

int Handle_SID02_Attribute_PictureSize()
{
    return 0;
}

int Handle_SID02_Attribute_Compressibility()
{
    return 0;
}

int Handle_SID02_Attribute_PictureLuminance()
{
    return 0;
}

int Handle_SID02_Attribute_PictureArguments()
{
    return 0;
}

int Handle_SID02_Attribute_TimeStamp()
{
    return 0;
}

int Handle_SID02_Attribute_GyroscopeAngle()
{
    return 0;
}

int Handle_SID02_Attribute_GyroscopeDirection()
{
    return 0;
}

int Handle_SID02_Attribute_GyroscopeConfig()
{
    return 0;
}

int Handle_SID02_Attribute_GyroscopeOpenning()
{
    return 0;
}

int Handle_SID02_Attribute_GyroscopeStorePicture()
{
    return 0;
}

int Handle_SID02_Attribute_HeatingWire()
{
return 0;
}

void build_response(enum SID02_Attribute item, int response_code, int num_args, ...)
{
    char response[] = {};
    int index = 0;
    response[index++] = 0xAA;
    response[index++] = 0x5A;
    response[index++] = 0x01;
    response[index++] = 0x02;
    response[index++] = 0x07;
    response[index++] = 0x00;
    response[index++] = 0x00;
    response[index++] = 0x00;
    response[index++] = item;
    response[index++] = response_code;
    va_list args;
    va_start(args, num_args);
     for (int i = 0; i < num_args; i++) 
     {
        int arg = va_arg(args, int);
        response[index++] = arg;
        log_i("Arg %d: %d\n", i + 1, arg);
    }
    va_end(args);
    while(index != 15)
    {
        response[index++] = 0x00;
    }
    send_msg_resp(response, index + 2);
}
