#include "cm_common.h"
#include "stdbool.h"
#include "SID03_SetAttribute.h"
#include "Cam485ProtocolCommon.h"
#include "ImageInfoList.h"
#include "elog.h"
#define LOG_TAG "[SID03_SetAttribute]"
// 定义错误码
#define ERR_SUCCESS 0
#define ERR_INVALID_PARAMS -1
#define ERR_SYSTEM_ERROR -2

// 属性处理函数的类型定义
typedef int (*AttributeHandler)(const uint8_t* value, uint32_t value_len);

// 属性表项结构
typedef struct {
    uint8_t id;
    AttributeHandler handler;
    uint32_t expected_value_len;  // 期望的值长度
} AttributeEntry;

// 属性处理函数声明
static int handle_slave_address(const uint8_t* value, uint32_t value_len);
static int handle_firmware_version(const uint8_t* value, uint32_t value_len);
static int handle_camera_num(const uint8_t* value, uint32_t value_len);
static int handle_baudrate(const uint8_t* value, uint32_t value_len);
static int handle_picture_size(const uint8_t* value, uint32_t value_len);
static int handle_compressibility(const uint8_t* value, uint32_t value_len);
static int handle_picture_luminance(const uint8_t* value, uint32_t value_len);
static int handle_picture_arguments(const uint8_t* value, uint32_t value_len);
static int handle_time_stamp(const uint8_t* value, uint32_t value_len);
static int handle_gyroscope_angle(const uint8_t* value, uint32_t value_len);
static int handle_gyroscope_direction(const uint8_t* value, uint32_t value_len);
static int handle_gyroscope_enable_status(const uint8_t* value, uint32_t value_len);
static int handle_gyroscope_store_picture(const uint8_t* value, uint32_t value_len);
static int handle_heating_wire(const uint8_t* value, uint32_t value_len);

static const AttributeEntry ATTRIBUTE_TABLE[] = {
    {SID03_Attribute_SloveAddress,    handle_slave_address,     1}, // 从地址
    {SID03_Attribute_FirmwareVersion, handle_firmware_version,  1}, // 固件版本
    {SID03_Attribute_CameraNum,       handle_camera_num,        2}, // 摄像头编号
    {SID03_Attribute_Baudrate,        handle_baudrate,          2}, // 波特率
    {SID03_Attribute_PictureSize,     handle_picture_size,      4}, // 图片尺寸
    {SID03_Attribute_Compressibility, handle_compressibility,   2}, // 压缩率
    {SID03_Attribute_PictureLuminance,handle_picture_luminance, 2}, // 图片亮度
    {SID03_Attribute_PictureArguments,handle_picture_arguments, 2}, // 图片参数
    {SID03_Attribute_TimeStamp,       handle_time_stamp,        0}, // 时间戳
    {SID03_Attribute_GyroscopeAngle,  handle_gyroscope_angle,   2}, // 陀螺仪角度
    {SID03_Attribute_GyroscopeDirection, handle_gyroscope_direction, 1}, // 陀螺仪方向
    {SID03_Attribute_GyroscopeOpenning, handle_gyroscope_enable_status, 1}, // 陀螺仪启用状态
    {SID03_Attribute_GyroscopeStorePicture, handle_gyroscope_store_picture, 2}, // 陀螺仪存储图片
    {SID03_Attribute_HeatingWire,     handle_heating_wire,      1}, // 加热丝
};

static const size_t ATTRIBUTE_TABLE_SIZE = sizeof(ATTRIBUTE_TABLE) / sizeof(AttributeEntry);

// 查找属性处理函数
static const AttributeEntry* find_attribute_handler(uint8_t id) {
    for (size_t i = 0; i < ATTRIBUTE_TABLE_SIZE; i++) {
        if (ATTRIBUTE_TABLE[i].id == id) {
            return &ATTRIBUTE_TABLE[i];
        }
    }
    return NULL;
}

// 验证消息格式
static int validate_message(const uint8_t* msg_buf, uint32_t msg_dlc) {
    if (msg_buf == NULL || msg_dlc < 10) {
        log_e("Invalid message parameters");
        return ERR_INVALID_PARAMS;
    }

    if (msg_buf[0] != 0xAA || msg_buf[1] != 0x5A) {
        log_e("Invalid frame header");
        return ERR_INVALID_PARAMS;
    }

    uint32_t len = (msg_buf[4]) | (msg_buf[5] << 8) | (msg_buf[6] << 16) | (msg_buf[7] << 24);
    if (len != msg_dlc - 10) {
        log_e("Data length mismatch");
        return ERR_INVALID_PARAMS;
    }

    return ERR_SUCCESS;
}
static void buildResponse(uint8_t *response, uint8_t item, int response_code) {
    int index = 0;
    response[index++] = 0xAA;
    response[index++] = 0x5A;
    response[index++] = 0x01;
    response[index++] = 0x03;
    response[index++] = 0x02;
    response[index++] = 0x00;
    response[index++] = 0x00;
    response[index++] = 0x00;
    response[index++] = item;
    response[index++] = response_code;
}
// 主处理函数
int SID03_SetAttribute(uint8_t *msg_buf, uint32_t msg_dlc) {
    log_i("SID03_SetAttribute");
    int result = validate_message(msg_buf, msg_dlc);
    if (result != ERR_SUCCESS) {
        return result;
    }

    uint8_t id = msg_buf[8];
    const uint8_t* value = &msg_buf[9];
    /*total length - 4 header -4 data length - 2 crc - 2id*/
    uint32_t value_len = msg_dlc - 10 -1;
    log_i("id: %d", id);
    log_i("value_len: %d", value_len);
    
    const AttributeEntry* entry = find_attribute_handler(id);
    if (!entry) {
        log_e("Unknown attribute ID: 0x%02X", id);
        return ERR_INVALID_PARAMS;
    }
    
    if (value_len < entry->expected_value_len) {
        log_e("Value length too short for attribute 0x%02X", id);
        return ERR_INVALID_PARAMS;
    }
    log_i("process attribute id: %d", id);
    result = entry->handler(value, value_len);
    
    uint8_t response[12] = {0};
    buildResponse(response, id, result);
    send_msg_resp(response, 12);
    
    return ERR_SUCCESS;
}

// 属性处理函数实现
static int handle_slave_address(const uint8_t* value, uint32_t value_len) {
    Set_g_slave_address(value[0]);
    return ERR_SUCCESS;
}

static int handle_firmware_version(const uint8_t* value, uint32_t value_len) {
    FILE *fp = fopen(VERSION_FILE, "w");
    if (fp == NULL) {
        log_e("Failed to open version file");
        return ERR_SYSTEM_ERROR;
    }
    
    fprintf(fp, "Version: %s\n", value[0]);
    fclose(fp);
    return ERR_SUCCESS;
}

static int handle_camera_num(const uint8_t* value, uint32_t value_len) {
    // Implement camera number handling logic
    return ERR_SUCCESS;
}

static int handle_baudrate(const uint8_t* value, uint32_t value_len) {
    // Implement baudrate handling logic
    return ERR_SUCCESS;
}

static int handle_picture_size(const uint8_t* value, uint32_t value_len) {
    uint16_t width = value[0] | (value[1] << 8);
    uint16_t height = value[2] | (value[3] << 8);
    int ret = wind_sample_set_resolution(width, height);
    Set_Camera_config(width, height);
    return ret == 0 ? ERR_SUCCESS : ERR_SYSTEM_ERROR;
}

static int handle_compressibility(const uint8_t* value, uint32_t value_len) {
    // Implement compressibility handling logic
    return ERR_SUCCESS;
}

static int handle_picture_luminance(const uint8_t* value, uint32_t value_len) {
    Set_Luminance(value[0] | (value[1] << 8));
    return ERR_SUCCESS;
}

static int handle_picture_arguments(const uint8_t* value, uint32_t value_len) {
    // Implement picture arguments handling logic
    return ERR_SUCCESS;
}

static int handle_time_stamp(const uint8_t* value, uint32_t value_len) {
    // Implement time stamp handling logic
    return ERR_SUCCESS;
}

static int handle_gyroscope_angle(const uint8_t* value, uint32_t value_len) {
    // Implement gyroscope angle handling logic
    return ERR_SUCCESS;
}

static int handle_gyroscope_direction(const uint8_t* value, uint32_t value_len) {
    log_i("Set Gyroscope capture direction");
    Set_Gyroscope_Capture_image_direction(value[0]);
    return ERR_SUCCESS;
}

static int handle_gyroscope_enable_status(const uint8_t* value, uint32_t value_len) {
    log_i("Set Gyroscope enable status");
    Set_Gyroscope_Enable_Status(value[0]);
    return ERR_SUCCESS;
}

static int handle_gyroscope_store_picture(const uint8_t* value, uint32_t value_len) {
    Set_Gyroscope_Image_Save_Count(value[0] | (value[1] << 8));
    return ERR_SUCCESS;
}

static int handle_heating_wire(const uint8_t* value, uint32_t value_len) {
    printf("heating_wire_rate : %d\n", value[0]);
    static char buffer[16];  
    snprintf(buffer, sizeof(buffer), "%u", value[0]);  
    save_to_config("PWM_Config", "DUTY_CYCLE", buffer);
    save_to_config("PWM_Config", "GPIO_PIN", "49");
    save_to_config("PWM_Config", "FREQUENCY", "1000");
    write_ini();
    system("pkill -f pwm_start.sh");
    system("/system/init/pwm_start.sh &");
    return ERR_SUCCESS;
}

