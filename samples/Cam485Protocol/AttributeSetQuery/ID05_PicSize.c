#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "elog.h"
#include "AttributeTable.h"
#include "cm_config.h"

static int handle_picture_size(const uint8_t* value, uint32_t value_len) {
    uint16_t width = value[0] | (value[1] << 8);
    uint16_t height = value[2] | (value[3] << 8);
    int ret = wind_sample_set_resolution(width, height);
    Set_Camera_config(width, height);
    return ret;
}

int attribute_picture_size_set(const uint8_t* value, uint32_t value_len)
{
    return 0 ;
}
int attribute_picture_size_get()
{
    return 0;
}