#ifndef SID02_QUERYATTRIBUTE
#define SID02_QUERYATTRIBUTE

#include <stdio.h>
#include <stdint.h>
#include <elog.h>
#include <stdarg.h>
#include "cm_config.h"

enum SID02_Attribute {
    SID02_Attribute_Unknown = 0x00,
    SID02_Attribute_SloveAddress = 0x01,
    SID02_Attribute_FirmwareVersion = 0x02,
    SID02_Attribute_CameraNum = 0x03,
    SID02_Attribute_Baudrate = 0x04,
    SID02_Attribute_PictureSize = 0x05,
    SID02_Attribute_Compressibility = 0x06,
    SID02_Attribute_PictureLuminance = 0x07,
    SID02_Attribute_PictureArguments = 0x08,
    SID02_Attribute_TimeStamp = 0x09,
    SID02_Attribute_GyroscopeAngle = 0x0A,
    SID02_Attribute_GyroscopeDirection = 0x0B,
    SID02_Attribute_GyroscopeConfig = 0x0C,
    SID02_Attribute_GyroscopeOpenning = 0x0D,
    SID02_Attribute_GyroscopeStorePicture = 0x0E,
    SID02_Attribute_HeatingWire = 0x0F
};

int SID02_QueryAttribute(uint8_t *msg_buf, uint32_t msg_dlc);

int Handle_SID02_Attribute_SloveAddress();
int Handle_SID02_Attribute_FirmwareVersion();
int Handle_SID02_Attribute_CameraNum();
int Handle_SID02_Attribute_Baudrate();
int Handle_SID02_Attribute_PictureSize();
int Handle_SID02_Attribute_Compressibility();
int Handle_SID02_Attribute_PictureLuminance();
int Handle_SID02_Attribute_PictureArguments();
int Handle_SID02_Attribute_TimeStamp();
int Handle_SID02_Attribute_GyroscopeAngle();
int Handle_SID02_Attribute_GyroscopeDirection();
int Handle_SID02_Attribute_GyroscopeConfig();
int Handle_SID02_Attribute_GyroscopeOpenning();
int Handle_SID02_Attribute_GyroscopeStorePicture();
int Handle_SID02_Attribute_HeatingWire();
void build_response(enum SID02_Attribute item, int response_code, int num_args, ...);

#endif