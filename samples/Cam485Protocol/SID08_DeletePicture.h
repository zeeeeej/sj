#ifndef _SEMDPHOTO_H_
#define _SEMDPHOTO_H_

#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include "sys/stat.h"
#include "unistd.h"
#include "circular_log.h"
#include "MsgDispatcher.h"
#include "cm_common.h"
#include "SID09_GetPhoto.h"
#include "ImageInfoList.h"

#define MD5_SIZE 16
#define MAX_PIC_NUM 10

#define TAKE_PHOTO_TMP_FILE "/tmp/act_trigger/image_"

int checkDeletePhoto(const uint8_t *msg_buf, uint16_t msg_len);
int HandleDeletePhoto(uint8_t *msg_buf, uint16_t msg_len);
int DeletePicture(const uint8_t pic_id);


#endif // _SEMDPHOTO_H_