#ifndef __HD_OTA_H__
#define __HD_OTA_H__

#include <stdint.h>

int hd_ota_stop_poll();

int hd_ota_poll_file();

int hd_read_file(uint8_t *buf,int file_size);

#endif