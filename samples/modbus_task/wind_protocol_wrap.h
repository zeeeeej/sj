//
// Created by DELL on 24-9-20.
//

#ifndef SAMPLES_WIND_PROTOCOL_WRAP_H
#define SAMPLES_WIND_PROTOCOL_WRAP_H

typedef enum {
    WIND_PROTOCOL_CMD_NONE = 0x00,
    WIND_PROTOCOL_CMD_GET_VERSION = 0xD1,
    WIND_PROTOCOL_CMD_GET_IMAGE = 0xD2,
    WIND_PROTOCOL_CMD_SET_ANGLE_OPEN = 0xD3,
    WIND_PROTOCOL_CMD_SET_ANGLE_IMAGE = 0xD4,
    WIND_PROTOCOL_CMD_SET_OPEN_TIMEOUT = 0xD5,
    WIND_PROTOCOL_CMD_SET_USB_CONNECT = 0xD6,

    WIND_PROTOCOL_CMD_REPORT_DOOR_OPEN = 0xF1,
    WIND_PROTOCOL_CMD_REPORT_DOOR_CLOSE = 0xF2,

    WIND_PROTOCOL_HEAD_PREFIX = 0x7A,
    WIND_PROTOCOL_HEAD_RESP_PREFIX = 0x9A,

    WIND_PROTOCOL_CMD_SUB_SET = 0x01,

    WIND_PROTOCOL_CMD_E_OK = 0xE0,
    WIND_PROTOCOL_CMD_E1 = 0xE1,
    WIND_PROTOCOL_CMD_E2 = 0xE2,
    WIND_PROTOCOL_CMD_E3 = 0xE3,
} WindProtocolCmd;

enum {
    WIND_PROTOCOL_IMAGE_TRANSFER_START = 0x00,
    WIND_PROTOCOL_IMAGE_TRANSFER_END = 0x01,
};

int wind_protocol_is_head(unsigned char *data, size_t len);

int wind_protocol_is_resp_head(unsigned char *data, size_t len);

int wind_protocol_get_cmd(unsigned char *data, size_t len);

int wind_protocol_is_cmd_get_version(unsigned char *data, size_t len);

int wind_protocol_is_cmd_get_image(unsigned char *data, size_t len);

int wind_protocol_is_cmd_set_angle_open(unsigned char *data, size_t len);

int wind_protocol_is_cmd_set_angle_image(unsigned char *data, size_t len);

int wind_protocol_is_cmd_set_open_timeout(unsigned char *data, size_t len);

int wind_protocol_is_cmd_set_usb_connect(unsigned char *data, size_t len);

int wind_protocol_get_sub_cmd(unsigned char *data, size_t len);

int wind_protocol_is_sub_cmd_query(unsigned char *data, size_t len);

int wind_protocol_is_sub_cmd_set(unsigned char *data, size_t len);

int wind_protocol_resp_version(unsigned char *data, size_t len);

int wind_protocol_resp_report_door_open(unsigned char *data, size_t len, unsigned long long open_count);

int wind_protocol_resp_report_door_close(unsigned char *data, size_t len, int angle, unsigned long long open_count);

int wind_protocol_get_angle(unsigned char *data, size_t len);


int wind_protocol_get_open_timeout(unsigned char *data, size_t len);

int wind_protocol_get_open_count(unsigned char *data, size_t len);

int wind_protocol_set_resp(unsigned char *data, size_t len, int set_type, int set_sub_type, int ok);

int wind_protocol_get_resp_angle_open(unsigned char *data, size_t len, int angle);

int wind_protocol_get_resp_angle_image(unsigned char *data, size_t len, int angle);

int wind_protocol_get_resp_door_timeout(unsigned char *data, size_t len, int timeout);

int wind_protocol_transfer_image_start_resp(unsigned char *data, size_t len, size_t image_len);

int wind_protocol_transfer_image_end_resp(unsigned char *data, size_t len);

int wind_protocol_transfer_image_resp_fail(unsigned char *data, size_t len);


#endif //SAMPLES_WIND_PROTOCOL_WRAP_H
