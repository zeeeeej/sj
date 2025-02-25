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
    uint8_t result = 0;
    char sn[25] = {0}; /*24 sn + 1 '\0'*/
    int sn_len = sizeof(sn);
    int ret = Get_g_CameraSn(sn, sizeof(sn));
    if (ret < 0)
    {
        log_e("Failed to get camera sn");
        result = 1;
    }   
    return SendGetAttributeResp(0x03, result, sn, sn_len);
}


