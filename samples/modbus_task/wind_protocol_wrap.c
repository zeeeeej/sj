//
// Created by DELL on 24-9-20.
//

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <cm_common.h>

#include "wind_protocol_wrap.h"
#include "wind_global.h"
#include "jkbytes.h"

int wind_protocol_is_head(unsigned char *data, size_t len)
{
    if (len < 1)
    {
        return -1;
    }
    return data[0] == WIND_PROTOCOL_HEAD_PREFIX;
}

int wind_protocol_is_resp_head(unsigned char *data, size_t len)
{
    if (len < 1)
    {
        return -1;
    }
    return data[0] == WIND_PROTOCOL_HEAD_RESP_PREFIX;
}

int wind_protocol_get_cmd(unsigned char *data, size_t len)
{
    if (len < 2)
    {
        return -1;
    }
    return data[0];
}

int wind_protocol_is_cmd_get_version(unsigned char *data, size_t len)
{
    if (len < 1)
    {
        return -1;
    }
    return data[0] == WIND_PROTOCOL_CMD_GET_VERSION;
}

int wind_protocol_is_cmd_get_image(unsigned char *data, size_t len)
{
    if (len < 1)
    {
        return -1;
    }
    return data[0] == WIND_PROTOCOL_CMD_GET_IMAGE;
}

int wind_protocol_is_cmd_set_angle_open(unsigned char *data, size_t len)
{
    if (len < 1)
    {
        return -1;
    }
    return data[0] == WIND_PROTOCOL_CMD_SET_ANGLE_OPEN;
}

int wind_protocol_is_cmd_set_angle_image(unsigned char *data, size_t len)
{
    if (len < 1)
    {
        return -1;
    }
    return data[0] == WIND_PROTOCOL_CMD_SET_ANGLE_IMAGE;
}

int wind_protocol_is_cmd_set_open_timeout(unsigned char *data, size_t len)
{
    if (len < 1)
    {
        return -1;
    }
    return data[0] == WIND_PROTOCOL_CMD_SET_OPEN_TIMEOUT;
}

int wind_protocol_is_cmd_set_usb_connect(unsigned char *data, size_t len)
{
    if (len < 1)
    {
        return -1;
    }
    return data[0] == WIND_PROTOCOL_CMD_SET_USB_CONNECT;
}

int wind_protocol_get_sub_cmd(unsigned char *data, size_t len)
{
    return 0;
}

int wind_protocol_is_sub_cmd_query(unsigned char *data, size_t len)
{
    if (len < 1)
    {
        return -1;
    }
    return data[0] == WIND_PROTOCOL_CMD_NONE;
}

int wind_protocol_is_sub_cmd_set(unsigned char *data, size_t len)
{
    if (len < 1)
    {
        return -1;
    }
    return data[0] == WIND_PROTOCOL_CMD_SUB_SET;
}

int wind_protocol_resp_version(unsigned char *data, size_t len)
{
    if (len < 6)
    {
        return -1;
    }
    data[0] = WIND_PROTOCOL_HEAD_RESP_PREFIX;
    data[1] = WIND_PROTOCOL_CMD_GET_VERSION;
    data[2] = WIND_PROTOCOL_CMD_NONE;

    int x = 0, y = 0, z = 0;
    sscanf(VERSION, "%d.%d.%d", &x, &y, &z);
    data[3] = (unsigned char)x;
    data[4] = (unsigned char)y;
    data[5] = (unsigned char)z;
    return 5;
}

int wind_protocol_resp_report_door_open(unsigned char *data, size_t len, unsigned long long open_count)
{
    if (len < 10)
    {
        return -1;
    }
    data[0] = WIND_PROTOCOL_HEAD_PREFIX;
    data[1] = WIND_PROTOCOL_CMD_REPORT_DOOR_OPEN;
    data[2] = WIND_PROTOCOL_CMD_NONE;
    save_int_to_hex((int)open_count, data + 3);
    return 7;
}

int wind_protocol_resp_report_door_close(unsigned char *data, size_t len, int angle, unsigned long long open_count)
{
    if (len < 10)
    {
        return -1;
    }
    data[0] = WIND_PROTOCOL_HEAD_PREFIX;
    data[1] = WIND_PROTOCOL_CMD_REPORT_DOOR_CLOSE;
    data[2] = WIND_PROTOCOL_CMD_NONE;
    save_int_to_hex((int)open_count, data + 3);
    data[7] = (unsigned char)angle;
    return 8;
}

int wind_protocol_get_angle(unsigned char *data, size_t len)
{
    if (len < 1)
    {
        return -1;
    }
    return (int)data[0];
}

int wind_protocol_get_open_timeout(unsigned char *data, size_t len)
{
    if (len < 2)
    {
        return -1;
    }
    int timeout = 300;
    memcpy(&timeout, data, 2);
    return timeout;
}

int wind_protocol_get_open_count(unsigned char *data, size_t len)
{
    if (len < 4)
    {
        return -1;
    }
    int open_count = 0;
    memcpy(&open_count, data, 4);
    return open_count;
}

int wind_protocol_set_resp(unsigned char *data, size_t len, int set_type, int set_sub_type, int ok)
{
    if (len < 4)
    {
        return -1;
    }
    data[0] = WIND_PROTOCOL_HEAD_RESP_PREFIX;
    data[1] = set_type;
    data[2] = set_sub_type;
    data[3] = ok ? WIND_PROTOCOL_CMD_E_OK : WIND_PROTOCOL_CMD_E1;
    return 4;
}

int wind_protocol_get_resp_angle_open(unsigned char *data, size_t len, int angle)
{
    if (len < 8)
    {
        return -1;
    }
    data[0] = WIND_PROTOCOL_HEAD_RESP_PREFIX;
    data[1] = WIND_PROTOCOL_CMD_SET_ANGLE_OPEN;
    data[2] = WIND_PROTOCOL_CMD_NONE;
    data[3] = WIND_PROTOCOL_CMD_E_OK;
    data[4] = (unsigned char)angle;
    return 5;
}

int wind_protocol_get_resp_angle_image(unsigned char *data, size_t len, int angle)
{
    if (len < 8)
    {
        return -1;
    }
    data[0] = WIND_PROTOCOL_HEAD_RESP_PREFIX;
    data[1] = WIND_PROTOCOL_CMD_SET_ANGLE_IMAGE;
    data[2] = WIND_PROTOCOL_CMD_NONE;
    data[3] = WIND_PROTOCOL_CMD_E_OK;
    data[4] = (unsigned char)angle;
    return 5;
}

int wind_protocol_get_resp_door_timeout(unsigned char *data, size_t len, int timeout)
{
    if (len < 8)
    {
        return -1;
    }
    data[0] = WIND_PROTOCOL_HEAD_RESP_PREFIX;
    data[1] = WIND_PROTOCOL_CMD_SET_OPEN_TIMEOUT;
    data[2] = WIND_PROTOCOL_CMD_NONE;
    data[3] = WIND_PROTOCOL_CMD_E_OK;
    save_short_to_hex(timeout, data + 4);
    return 6;
}

int wind_protocol_transfer_image_start_resp(unsigned char *data, size_t len, size_t image_len)
{
    if (len < 8)
    {
        return -1;
    }
    data[0] = WIND_PROTOCOL_HEAD_RESP_PREFIX;
    data[1] = WIND_PROTOCOL_CMD_GET_IMAGE;
    data[2] = WIND_PROTOCOL_IMAGE_TRANSFER_START;
    data[3] = WIND_PROTOCOL_CMD_E_OK;
    save_int_to_hex((int)image_len, data + 4);
    return 8;
}

int wind_protocol_transfer_image_end_resp(unsigned char *data, size_t len)
{
    if (len < 8)
    {
        return -1;
    }
    data[0] = WIND_PROTOCOL_HEAD_RESP_PREFIX;
    data[1] = WIND_PROTOCOL_CMD_GET_IMAGE;
    data[2] = WIND_PROTOCOL_IMAGE_TRANSFER_END;
    data[3] = WIND_PROTOCOL_CMD_E_OK;
    return 4;
}

int wind_protocol_transfer_image_resp_fail(unsigned char *data, size_t len)
{
    if (len < 4)
    {
        return -1;
    }
    data[0] = WIND_PROTOCOL_HEAD_RESP_PREFIX;
    data[1] = WIND_PROTOCOL_CMD_GET_IMAGE;
    data[2] = WIND_PROTOCOL_CMD_NONE;
    data[3] = WIND_PROTOCOL_CMD_E1;
    return 4;
}

#ifdef __TEST_MAIN__
int main(int argc, char *argv[])
{
    unsigned char data[10] = {0};
    wind_protocol_resp_version(data, 10);
    LOGD("1. [%x %x %x %x %x %x]\n", data[0], data[1], data[2], data[3], data[4], data[5]);

    wind_protocol_resp_report_door_open(data, 10, 2147483647);
    LOGD("2. [%x %x %x %x %x %x %x %x %x]\n", data[0], data[1], data[2], data[3], data[4], data[5],
         data[6], data[7], data[8]);

    int avalue = 0;
    memcpy(&avalue, data + 3, 4);
    LOGD("recovery int value: %d, %d\n", avalue, INT_MAX);

    data[0] = 0x26;
    data[1] = 0x00;
    int angle = wind_protocol_get_angle(data, 10);
    LOGD("angle = [%d]\n", angle);

    wind_protocol_get_resp_door_timeout(data, 10, 3600);
    LOGD("3. [%x %x %x %x %x %x]\n", data[0], data[1], data[2], data[3], data[4], data[5]);
    int timeout = 0;
    memcpy(&timeout, data + 4, 2);
    LOGD("timeout = [%d]\n", timeout);

    wind_protocol_transfer_image_resp(data, 10, 0, 2*1024*1024);
    LOGD("4. [%x %x %x %x %x %x %x %x]\n", data[0], data[1], data[2], data[3], data[4],
         data[5], data[6], data[7]);
    int image_len = 0;
    memcpy(&image_len, data + 4, 4);
    LOGD("image_len = [%d]\n", image_len);
    return 0;
}
#endif
