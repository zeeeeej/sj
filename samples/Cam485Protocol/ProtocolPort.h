#ifndef PROTOCOL_CFG_H__

#define ACTIVE_TRIGGER_PHOTO_FILE_DIR   "/tmp/act_trigger/"       /*主动拍照*/
#define GYRO_TRIGGER_PHOTO_FILE_DIR     "/tmp/gyro_trigger/"    /*陀螺仪触发*/


/*主动拍照图像文件命名格式 image_id  例如： image_1   image_2*
陀螺仪触发拍照 命名格式  image_id_angle  image_1_30  image_2_24*/

#define ACTIVE_TRIGGER_TYPE     0x01
#define GYRO_TRIGGER_TYPE       0x00
#define TRIGGER_TYPE_UNKNOWN    -1


#define SLAVE_ADDR          0x01


#define PACK_HEADER 
#define PACK_HEADER_LEN         

#define PACKET_TOTAL_LEN        
#include "stdint.h"
uint8_t request_take_photo(char *file);
int Protocol_Init();
uint8_t get_image_seq();
#endif