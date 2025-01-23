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

void Set_g_slave_address(uint16_t slave_address);
void Set_Camera_config(uint16_t width, uint16_t height);
void Get_g_slave_address();
void Get_Camera_config();

void LoadConfig();

#endif