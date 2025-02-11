#ifndef _CM_CONFIG_H_
#define _CM_CONFIG_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include "ParseIni.h"
#include "sample-common.h"
#include <elog.h>

extern uint16_t g_slave_address;
#define VERSION_FILE "/system/etc/version"

void Set_g_slave_address(uint16_t slave_address);
void Set_Camera_config(uint16_t width, uint16_t height);
void Set_Luminance(uint16_t luminance);
const char * Get_g_slave_address();
const char * Get_Camera_config();
const char *ReadFirware();
const char * getLuminance();

/*陀螺仪设置*/
void Set_Gyroscope_Capture_image_direction(uint8_t direction);
uint8_t  Get_Gyroscope_Capture_image_direction();
void Set_Gyroscope_Enable_Status(uint8_t Enable_Status);
uint8_t Get_Gyroscope_Enable_Status(uint8_t *Enable_Status);
void Set_Gyroscope_Image_Save_Count(uint8_t SaveCount);
uint8_t Get_Gyroscope_Image_Save_Count();
/*陀螺仪设置*/


void LoadConfig();

#endif