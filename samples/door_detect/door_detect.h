#ifndef DOOR_DETECT_PRO
#define DOOR_DETECT_PRO
void *hmi_service_thread(void *args); 
int door_detect_init();
int door_detect_deinit();
char get_door_status();
#endif