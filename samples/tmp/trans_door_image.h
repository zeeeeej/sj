#ifndef __TRANS_DOOR_IMAGE_H__
#define __TRANS_DOOR_IMAGE_H__


#define TAKE_PHOTO_BUFFER_SIZE 8192
int handle_trans_door_photo_request(char *response, int response_max_len, const char *request_data, int request_len);

#endif