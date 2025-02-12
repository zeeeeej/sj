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

void Set_g_slave_address(uint8_t slave_address);
void Set_g_firmwareVersion(uint32_t firmwareVersion);
void Set_g_CameraNum(uint32_t CameraNum);
void Set_g_Rs485Baudrate(uint32_t CameraNum);
uint8_t Get_slave_address();
uint32_t Get_Firwareversion();
uint32_t Get_CameraNum();
uint32_t Get_Rs485Baudrate();
void Set_Camera_config(uint16_t width, uint16_t height);
void Set_Luminance(uint16_t luminance);
// const char * Get_g_slave_address();
const char * Get_Camera_config();
const char *ReadFirware();
const char * getLuminance();

void LoadConfig();

#endif