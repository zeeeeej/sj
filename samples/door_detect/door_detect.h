#ifndef DOOR_DETECT_PRO
#define DOOR_DETECT_PRO
void *hmi_service_thread(void *args); 
int door_init();
int door_deinit();
char get_door_status();
#endif