#ifndef DOOR_DETECT_PRO
#define DOOR_DETECT_PRO
#include "stdint.h"
int door_detect_init();
int door_detect_deinit();
char get_door_status();
int set_gyroscope_capture_direction(uint8_t capture_direction);
int set_gyroscope_capture_angle(uint8_t capture_angle);
#endif
