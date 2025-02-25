#include "stdio.h"

#include "Cam485ProtocolCommon.h"
#include "Cam485Protocol.h"
#include "MsgDispatcher.h"
#include "AttributeTable.h"
#include "elog.h"
#define TAG_NAME "[ATTRIBUTE_TABLE]"


static const AttributeEntry ATTRIBUTE_TABLE[] = {
    {
        SID03_Attribute_SloveAddress,        // 从地址
        attribute_slave_address_set,
        attribute_slave_address_get,
        1
    },
    {
        SID03_Attribute_FirmwareVersion,     // 固件版本
        attribute_firmware_version_set,
        attribute_firmware_version_get,
        1
    },
    {
        SID03_Attribute_CameraNum,           // 摄像头编号
        attribute_camera_number_set,
        attribute_camera_number_get,
        2
    },
    {
        SID03_Attribute_Baudrate,            // 波特率
        attribute_baudrate_set,
        attribute_baudrate_get,
        2
    },
    {
        SID03_Attribute_PictureSize,         // 图片尺寸
        attribute_picture_size_set,
        attribute_picture_size_get,
        4
    },
    {
        SID03_Attribute_Compressibility,     // 压缩率
        attribute_compression_ratio_set,
        attribute_compression_ratio_get,
        1
        
    },
    {
        SID03_Attribute_PictureLuminance,    // 图片亮度
        attribute_picture_brightness_set,
        attribute_picture_brightness_get,
        1
    },
    {
        SID03_Attribute_PictureArguments,    // 图片参数
        // attribute_picture_params_set,
        // attribute_picture_params_get,
        // 2
    },
    {
        SID03_Attribute_TimeStamp,           // 时间戳
        attribute_timestamp_set,
        attribute_timestamp_get,
        0
    },
    {
        SID03_Attribute_GyroscopeAngle,      // 陀螺仪角度
        attribute_gyro_angle_set,
        attribute_gyro_angle_get,
        2
    },
    {
        SID03_Attribute_GyroscopeDirection,  // 陀螺仪方向
        attribute_gyro_direction_set,
        attribute_gyro_direction_get,
        1
    },
    {
        SID03_Attribute_GyroscopeConfig,     // 陀螺仪配置
        // attribute_gyro_config_set,
        // attribute_gyro_config_get,
        // 1
    },
    {
        SID03_Attribute_GyroscopeOpenning,   // 陀螺仪启用状态
        attribute_gyro_enable_set,
        attribute_gyro_enable_get,
        1
    },
    {
        SID03_Attribute_GyroscopeStorePicture, // 陀螺仪存储图片
        attribute_gyro_store_picture_set,
        attribute_gyro_store_picture_get,
        2
    },
    {
        SID03_Attribute_HeatingWire,         // 加热丝
        attribute_heating_wire_set,
        attribute_heating_wire_get,
        1
    }
};
static const size_t ATTRIBUTE_TABLE_SIZE = sizeof(ATTRIBUTE_TABLE) / sizeof(AttributeEntry);

// 查找属性处理函数
const AttributeEntry* find_attribute_handler(uint8_t id) {
    for (size_t i = 0; i < ATTRIBUTE_TABLE_SIZE; i++) {
        if (ATTRIBUTE_TABLE[i].id == id) {
            return &ATTRIBUTE_TABLE[i];
        }
    }
    return NULL;
}

int SendSetAttributeResp(uint8_t id, uint8_t result)
{   
    uint8_t resp_buf[12];
    uint32_t payload_length = 2;
    uint32_t resp_buf_length = sizeof(resp_buf);
    int index = 0;
    resp_buf[index++] = 0xAA;
    resp_buf[index++] = 0x5A;
    resp_buf[index++] = 0x01;
    resp_buf[index++] = 0x03;/*设置属性id*/
    
    resp_buf[index++] = (uint8_t)(payload_length & 0xFF);         // Low byte
    resp_buf[index++] = (uint8_t)((payload_length >> 8) & 0xFF);  // 2nd byte
    resp_buf[index++] = (uint8_t)((payload_length >> 16) & 0xFF); // 3rd byte
    resp_buf[index++] = (uint8_t)((payload_length >> 24) & 0xFF); // High byte

    resp_buf[index++] = id;
    resp_buf[index++] = result;

    return send_msg_resp(resp_buf,resp_buf_length);
}

int SendGetAttributeResp(uint8_t id, uint8_t result,uint8_t* value,uint32_t value_length)
{
    
    uint32_t payload_length = value_length + 1 +1 ;/*value length + id + reuslt*/
    uint8_t resp_buf[payload_length+4+4+2];
    uint32_t resp_buf_length = sizeof(resp_buf);
    int index = 0;
    resp_buf[index++] = 0xAA;
    resp_buf[index++] = 0x5A;
    resp_buf[index++] = 0x01;
    resp_buf[index++] = 0x02;/*查询属性id*/
    
    resp_buf[index++] = (uint8_t)(payload_length & 0xFF);         // Low byte
    resp_buf[index++] = (uint8_t)((payload_length >> 8) & 0xFF);  // 2nd byte
    resp_buf[index++] = (uint8_t)((payload_length >> 16) & 0xFF); // 3rd byte
    resp_buf[index++] = (uint8_t)((payload_length >> 24) & 0xFF); // High byte

    resp_buf[index++] = id;
    resp_buf[index++] = result;

    if (value != NULL && value_length > 0) {
        memcpy(&resp_buf[index], value, value_length);
        index += value_length;
    }

    return send_msg_resp(resp_buf,resp_buf_length);
}


