#ifndef __ATTRIBUTE_TABLE_H__
#define __ATTRIBUTE_TABLE_H__
#include "stdint.h"
enum SID03_Attribute {
    SID03_Attribute_Unknown = 0x00,
    SID03_Attribute_SloveAddress = 0x01,
    SID03_Attribute_FirmwareVersion = 0x02,
    SID03_Attribute_CameraNum = 0x03,
    SID03_Attribute_Baudrate = 0x04,
    SID03_Attribute_PictureSize = 0x05,
    SID03_Attribute_Compressibility = 0x06,
    SID03_Attribute_PictureLuminance = 0x07,
    SID03_Attribute_PictureArguments = 0x08,
    SID03_Attribute_TimeStamp = 0x09,
    SID03_Attribute_GyroscopeAngle = 0x0A,
    SID03_Attribute_GyroscopeDirection = 0x0B,
    SID03_Attribute_GyroscopeConfig = 0x0C,
    SID03_Attribute_GyroscopeOpenning = 0x0D,
    SID03_Attribute_GyroscopeStorePicture = 0x0E,
    SID03_Attribute_HeatingWire = 0x0F
};


// 属性处理函数的类型定义
typedef int (*AttributeSetHandler)(const uint8_t* value, uint32_t value_len);
typedef int (*AttributeGetHandler)();

// 属性表项结构
typedef struct {
    uint8_t id;                      // 属性ID
    AttributeSetHandler set_handler;  // 设置属性的处理函数
    AttributeGetHandler get_handler;  // 获取属性的处理函数
    uint32_t expected_value_len;     // 期望的值长度
} AttributeEntry;
const AttributeEntry* find_attribute_handler(uint8_t id);


int attribute_slave_address_set(const uint8_t* value, uint32_t value_len);
int attribute_slave_address_get();

int attribute_firmware_version_set(const uint8_t* value, uint32_t value_len);
int attribute_firmware_version_get();

int attribute_camera_number_set(const uint8_t* value, uint32_t value_len);
int attribute_camera_number_get();

int attribute_baudrate_set(const uint8_t* value, uint32_t value_len);
int attribute_baudrate_get();

int attribute_picture_size_set(const uint8_t* value, uint32_t value_len);
int attribute_picture_size_get();

int attribute_compression_ratio_set(const uint8_t* value, uint32_t value_len);
int attribute_compression_ratio_get();

int attribute_picture_brightness_set(const uint8_t* value, uint32_t value_len);
int attribute_picture_brightness_get();

int attribute_picture_params_set(const uint8_t* value, uint32_t value_len);
int attribute_picture_params_get();

int attribute_timestamp_set(const uint8_t* value, uint32_t value_len);
int attribute_timestamp_get();

int attribute_gyro_angle_set(const uint8_t* value, uint32_t value_len);
int attribute_gyro_angle_get();

int attribute_gyro_direction_set(const uint8_t* value, uint32_t value_len);
int attribute_gyro_direction_get();

int attribute_gyro_config_set(const uint8_t* value, uint32_t value_len);
int attribute_gyro_config_get();

int attribute_gyro_enable_set(const uint8_t* value, uint32_t value_len);
int attribute_gyro_enable_get();

int attribute_gyro_store_picture_set(const uint8_t* value, uint32_t value_len);
int attribute_gyro_store_picture_get();

int attribute_heating_wire_set(const uint8_t* value, uint32_t value_len);
int attribute_heating_wire_get();

int SendGetAttributeResp(uint8_t id, uint8_t result,uint8_t* value,uint32_t value_length);
int SendSetAttributeResp(uint8_t id, uint8_t result);



int Get_Gyroscope_Capture_image_direction(uint8_t *direction);
int Get_Gyroscope_Enable_Status(uint8_t *Enable_Status);
int Get_Gyroscope_Image_Save_Count(uint8_t *SaveCount);

int Get_g_timeStamp(uint32_t *timestamp);/*获取当前时间戳*/
int showDate(uint32_t _timestamp);
#endif
