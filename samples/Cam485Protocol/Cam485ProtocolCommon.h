#ifndef _CAM_485_PROTOCOL_CM_H__
#define _CAM_485_PROTOCOL_CM_H__

#include "stdint.h"
#include "elog.h"

#define ACTIVE_TRIGGER_PHOTO_FILE_DIR "/tmp/act_trigger/" /*主动拍照*/
#define GYRO_TRIGGER_PHOTO_FILE_DIR "/tmp/gyro_trigger/"  /*陀螺仪触发*/

/*主动拍照图像文件命名格式 image_id  例如： image_1   image_2*
陀螺仪触发拍照 命名格式  image_id_angle  image_1_30  image_2_24*/

#define ACTIVE_TRIGGER_TYPE 0x01
#define GYRO_TRIGGER_TYPE 0x00
#define TRIGGER_TYPE_UNKNOWN -1
// #define SLAVE_ADDR_DEFAULT 0x01




uint8_t get_image_seq();
uint8_t request_take_photo(char *file);
uint8_t query_door_status();
int remove_dir(const char *dir_path);
int check_and_create_dir(const char *dir_path);
int Get_Protocol_Slave_Address(uint8_t *slave_address);
int Set_Protocol_Slave_Address(uint8_t slave_address);

#endif
