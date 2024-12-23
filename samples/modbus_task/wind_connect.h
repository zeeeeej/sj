//
// Created by DELL on 24-9-20.
//

#ifndef SAMPLES_WIND_CONNECT_H
#define SAMPLES_WIND_CONNECT_H

#include "cm_config.h"

int wind_connect_init_in_thread();

int wind_connect_deinit();

int wind_connect_send_data(const char *data, int len);

int wind_connect_door_open(unsigned long long count);

int wind_connect_door_close(DoorInfoItem *item);

#endif //SAMPLES_WIND_CONNECT_H
