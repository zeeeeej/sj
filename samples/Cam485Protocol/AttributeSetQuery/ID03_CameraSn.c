#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "elog.h"
#include "AttributeTable.h"
#include "cm_config.h"

int attribute_camera_number_set(const uint8_t* value, uint32_t value_len)
{
    return 0;
}
int attribute_camera_number_get()
{
    char sn[24] = {0};
    int ret = Get_g_CameraSn(sn, sizeof(sn));
    if (ret < 0)
    {
        log_e("Failed to get camera sn");
        return SendGetAttributeResp(0x03, ret, ret, 0);
    }
    return SendGetAttributeResp(0x03, 0, sn, strlen(sn));
}


