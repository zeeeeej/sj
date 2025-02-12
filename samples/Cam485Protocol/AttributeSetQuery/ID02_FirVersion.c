#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "elog.h"
#include "AttributeTable.h"
#include "cm_config.h"
#define TAG_NAME "[ID02_FirVersion]"

int attribute_firmware_version_set(const uint8_t* value, uint32_t value_len)
{
    return 0;
}
int attribute_firmware_version_get()
{
    int ret = 0;
    uint8_t result = 0;
    char version_str[16] = {0};
    ret = Get_g_firmwareVersion(version_str, sizeof(version_str));
    if (ret < 0) {
        log_e("Failed to get firmware version");
        result = 1;
    }
    log_i("Firmware version: %s", version_str);
    uint8_t Getvalue[16] = {0};
    memcpy(Getvalue, version_str, sizeof(version_str));
    uint32_t ValueLen = sizeof(Getvalue);
    return SendGetAttributeResp(0x02,result,Getvalue,ValueLen);
}