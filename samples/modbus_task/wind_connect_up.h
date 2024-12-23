#ifndef __WIND_CONNECT_UP_H__
#define __WIND_CONNECT_UP_H__

#include "cm_config.h"

#define WIND_CONNECT_UP_DEBUG 1
int wind_connect_up_start();

int wind_connect_up_stop();

int wind_connect_up_door_open(unsigned long long count,float angle);

int wind_connect_up_door_close(DoorInfoItem *item);

#endif