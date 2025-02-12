#ifndef SID03_SETATTRIBUTE_ID1_4
#define SID03_SETATTRIBUTE_ID1_4

#include <stdio.h>
#include <elog.h>
#include <stdint.h>
#include "sample-common.h"
#include "ParseIni.h"
#include "cm_common.h"
#include "circular_log.h"
#include "cm_config.h"



int Handle_SID03_Attribute_SloveAddress(uint8_t *msg_buf, uint32_t msg_dlc);
int Handle_SID03_Attribute_FirmwareVersion(uint8_t *msg_buf, uint32_t msg_dlc);
int Handle_SID03_Attribute_CameraNum(uint8_t *msg_buf, uint32_t msg_dlc);
int Handle_SID03_Attribute_Baudrate(uint8_t *msg_buf, uint32_t msg_dlc);

void funtmp(uint8_t *msg_buf, uint32_t msg_dlc);

#endif