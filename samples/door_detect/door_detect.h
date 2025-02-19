#ifndef DOOR_DETECT_PRO
#define DOOR_DETECT_PRO
#include "stdint.h"
int door_detect_init();
int door_detect_deinit();
char get_door_status();
int set_capture_direction(uint8_t capture_direction);
#endif