#ifndef SID03_SETATTRIBUTE
#define SID03_SETATTRIBUTE

#include <stdio.h>
#include <elog.h>
#include <stdint.h>
#include "sample-common.h"
#include "ParseIni.h"
#include "cm_common.h"
#include "circular_log.h"


#define VERSION_FILE "/system/etc/version"



int SID03_SetAttribute(uint8_t *msg_buf, uint32_t msg_dlc);

#endif