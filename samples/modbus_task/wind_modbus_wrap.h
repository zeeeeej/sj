#ifndef __WIND_MODBUS_WRAP_H
#define __WIND_MODBUS_WRAP_H

#include "modbus/modbus.h"

int wind_modbus_wrap_init();

int wind_modbus_wrap_deinit();

int wind_modbus_wrap_recv(unsigned char *buf, int len);

int wind_modbus_wrap_send(unsigned char *buf, int len);

void modbus_rtu_custom_rts_control(modbus_t *ctx, int on);

#endif