#ifndef __TAKE_PHOTO_H__
#define __TAKE_PHOTO_H__


#define TAKE_PHOTO_TMP_DIR "/tmp/take_photo"
#define TAKE_PHOTO_TMP_FILE TAKE_PHOTO_TMP_DIR "/photo.jpg"
#define TAKE_PHOTO_BUFFER_SIZE 8192
int handle_photo_request(char *response, int response_max_len, const char *request_data, int request_len);

#endif