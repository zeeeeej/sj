#ifndef SID03_SETATTRIBUTE
#define SID03_SETATTRIBUTE

#include <stdio.h>
#include <elog.h>
#include <stdint.h>
#include "sample-common.h"
#include "ParseIni.h"
#include "cm_common.h"
#include "circular_log.h"
#include "cm_config.h"

enum SID03_Attribute {
    SID03_Attribute_Unknown = 0x00,
    SID03_Attribute_SloveAddress = 0x01,
    SID03_Attribute_FirmwareVersion = 0x02,
    SID03_Attribute_CameraNum = 0x03,
    SID03_Attribute_Baudrate = 0x04,
    SID03_Attribute_PictureSize = 0x05,
    SID03_Attribute_Compressibility = 0x06,
    SID03_Attribute_PictureLuminance = 0x07,
    SID03_Attribute_PictureArguments = 0x08,
    SID03_Attribute_TimeStamp = 0x09,
    SID03_Attribute_GyroscopeAngle = 0x0A,
    SID03_Attribute_GyroscopeDirection = 0x0B,
    SID03_Attribute_GyroscopeConfig = 0x0C,
    SID03_Attribute_GyroscopeOpenning = 0x0D,
    SID03_Attribute_GyroscopeStorePicture = 0x0E,
    SID03_Attribute_HeatingWire = 0x0F
};


#define VERSION_FILE "/system/etc/version"



int SID03_SetAttribute(uint8_t *msg_buf, uint32_t msg_dlc);

#endif