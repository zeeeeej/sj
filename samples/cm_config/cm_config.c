#include "cm_config.h"
#include "elog.h"
#define LOG_TAG  "[CM_CONFIG]"
/*******************陀螺仪设置********************/
/*获取抓图方向*/
int Get_Gyroscope_Capture_image_direction(uint8_t *direction)
{
    if (direction == NULL) {
        log_e("Invalid parameter: direction is NULL");
        return -1;
    }

    const char *value = get_config_value("Gyroscope", "CaptureDirection");
    if (value != NULL && strcmp(value, "NULL") != 0)
    {
        int temp_direction = atoi(value);
        // 确保值在合理范围内
        if (temp_direction >= 0 && temp_direction <= 255) {
            *direction = (uint8_t)temp_direction;
            log_i("Get Gyroscope Capture image direction: %d", *direction);
            return 0;
        } else {
            log_e("Invalid direction value: %d", temp_direction);
            return -1;
        }
    }
    else
    {
        log_e("CaptureDirection value is NULL!");
        return -1;
    }
}

/*设置抓图方向*/
int Set_Gyroscope_Capture_image_direction(uint8_t direction)
{
    log_i("Set Gyroscope_Capture_image_direction: %d", direction);
    
    char buffer[16];  
    snprintf(buffer, sizeof(buffer), "%u", direction); 
    
    if (save_to_config("Gyroscope", "CaptureDirection", buffer) != 0) {
        log_e("Failed to save CaptureDirection to config");
        return -1;
    }
    
    if (write_ini() != 0) {
        log_e("Failed to write config to file");
        return -1;
    }
    
    return 0;
}

/*设置陀螺仪开启状态*/
int Set_Gyroscope_Enable_Status(uint8_t Enable_Status)
{
    log_i("Set Gyroscope Enable Status: %d", Enable_Status);
    
    char buffer[16];  
    snprintf(buffer, sizeof(buffer), "%u", Enable_Status); 
    
    if (save_to_config("Gyroscope", "EnableStatus", buffer) != 0) {
        log_e("Failed to save EnableStatus to config");
        return -1;
    }
    
    if (write_ini() != 0) {
        log_e("Failed to write config to file");
        return -1;
    }
    return 0;
}

/*获取陀螺仪开启状态*/
int Get_Gyroscope_Enable_Status(uint8_t *Enable_Status)
{
    if (Enable_Status == NULL) {
        log_e("Invalid parameter: Enable_Status is NULL");
        return -1;
    }

    const char *value = get_config_value("Gyroscope", "EnableStatus");
    if (value != NULL && strcmp(value, "NULL") != 0) {
        int temp_status = atoi(value);
        // 确保值在合理范围内（0或1）
        if (temp_status == 0 || temp_status == 1) {
            *Enable_Status = (uint8_t)temp_status;
            log_i("Get Gyroscope Enable Status: %d", *Enable_Status);
            return 0;
        } else {
            log_e("Invalid enable status value: %d", temp_status);
            return -1;
        }
    } else {
        log_e("EnableStatus value is NULL!");
        return -1;
    }
}

/*设置陀螺仪抓图最多存储数量*/
int Set_Gyroscope_Image_Save_Count(uint8_t SaveCount)
{
    log_i("Set Gyroscope Capture image save count: %d", SaveCount);
    
    char buffer[16];  
    snprintf(buffer, sizeof(buffer), "%u", SaveCount); 
    
    if (save_to_config("Gyroscope", "SaveCount", buffer) != 0) {
        log_e("Failed to save SaveCount to config");
        return -1;
    }
    
    if (write_ini() != 0) {
        log_e("Failed to write config to file");
        return -1;
    }
    
    return 0;
}
/*获取陀螺仪抓图最多存储数量*/
int Get_Gyroscope_Image_Save_Count(uint8_t *SaveCount)
{
    if (SaveCount == NULL) {
        log_e("SaveCount pointer is NULL!");
        return 0;
    }

    const char *value = get_config_value("Gyroscope", "SaveCount");
    
    // 检查配置值是否有效
    if (value == NULL) {
        log_e("SaveCount value is NULL!");
        *SaveCount = 0;
        return -1;
    }
    
    // 转换字符串到整数
    char *endptr;
    long count = strtol(value, &endptr, 10);
    
    // 检查转换是否成功且值在有效范围内
    if (*endptr != '\0' || count < 0 || count > UINT8_MAX) {
        log_e("Invalid SaveCount value: %s", value);
        *SaveCount = 0;
        return -1;
    }
    
    *SaveCount = (uint8_t)count;
    log_i("Get Gyroscope Capture Image Save Count: %u", *SaveCount);
    return 0;
}


/*******************从机地址**********************/


/*设置从机地址*/
int Set_g_slave_address(uint8_t slave_address)
{
    log_i("Set slave address: %d", slave_address);
    
    char buffer[16];  
    snprintf(buffer, sizeof(buffer), "%u", slave_address); 
    
    if (save_to_config("slave_device", "slave_address", buffer) != 0) {
        log_e("Failed to save slave_address to config");
        return -1;
    }
    
    if (write_ini() != 0) {
        log_e("Failed to write config to file");
        return -1;
    }
    
    return 0;
}

/*获取从机地址*/
int Get_g_slave_address(uint8_t *slave_address)
{
    if (slave_address == NULL) {
        log_e("Invalid parameter: slave_address is NULL");
        return -1;
    }

    const char *value = get_config_value("slave_device", "slave_address");
    if (value != NULL && strcmp(value, "NULL") != 0) {
        int temp_address = atoi(value);
        // 确保值在合理范围内
        if (temp_address >= 0 && temp_address <= 255) {
            *slave_address = (uint8_t)temp_address;
            log_i("Get slave address: %d", *slave_address);
            return 0;
        } else {
            log_e("Invalid slave address value: %d", temp_address);
            return -1;
        }
    } else {
        log_e("slave_address value is NULL!");
        return -1;
    }
}

/*******************固件版本********************/
int Set_g_firmwareVersion(uint32_t firmwareVersion)
{
    log_i("Set firmware version: %u", firmwareVersion);
    
    char buffer[16];  
    snprintf(buffer, sizeof(buffer), "%u", firmwareVersion); 
    
    if (save_to_config("slave_device", "firmwareVersion", buffer) != 0) {
        log_e("Failed to save firmware version to config");
        return -1;
    }
    
    if (write_ini() != 0) {
        log_e("Failed to write config to file");
        return -1;
    }
    
    return 0;
}

/*获取固件版本*/
int Get_g_firmwareVersion(uint32_t *firmwareVersion)
{
    if (firmwareVersion == NULL) {
        log_e("Invalid parameter: firmwareVersion is NULL");
        return -1;
    }

    const char *value = get_config_value("slave_device", "firmwareVersion");
    if (value != NULL && strcmp(value, "NULL") != 0) {
        char *endptr;
        unsigned long version = strtoul(value, &endptr, 10);
        
        if (*endptr != '\0' || version > UINT32_MAX) {
            log_e("Invalid firmware version value: %s", value);
            return -1;
        }
        
        *firmwareVersion = (uint32_t)version;
        log_i("Get firmware version: %u", *firmwareVersion);
        return 0;
    } else {
        log_e("Firmware version value is NULL!");
        return -1;
    }
}






/*设置相机宽高配置*/
int Set_Camera_Resolution(uint16_t width, uint16_t height)
{
    log_i("Set camera resolution: %ux%u", width, height);
    
    char buffer[16];  
    
    // 设置宽度
    snprintf(buffer, sizeof(buffer), "%u", width); 
    if (save_to_config("camera", "width", buffer) != 0) {
        log_e("Failed to save camera width to config");
        return -1;
    }
    
    // 设置高度
    snprintf(buffer, sizeof(buffer), "%u", height); 
    if (save_to_config("camera", "height", buffer) != 0) {
        log_e("Failed to save camera height to config");
        return -1;
    }
    
    if (write_ini() != 0) {
        log_e("Failed to write config to file");
        return -1;
    }
    
    return 0;
}

/*获取相机宽高配置*/
int Get_Camera_Resolution(uint16_t *width, uint16_t *height)
{
    if (width == NULL || height == NULL) {
        log_e("Invalid parameter: width or height is NULL");
        return -1;
    }

    // 获取宽度
    const char *width_value = get_config_value("camera", "width");
    if (width_value == NULL || strcmp(width_value, "NULL") == 0) {
        log_e("Camera width value is NULL!");
        return -1;
    }

    // 获取高度
    const char *height_value = get_config_value("camera", "height");
    if (height_value == NULL || strcmp(height_value, "NULL") == 0) {
        log_e("Camera height value is NULL!");
        return -1;
    }

    // 转换宽度
    char *endptr;
    unsigned long temp_width = strtoul(width_value, &endptr, 10);
    if (*endptr != '\0' || temp_width > UINT16_MAX) {
        log_e("Invalid camera width value: %s", width_value);
        return -1;
    }

    // 转换高度
    unsigned long temp_height = strtoul(height_value, &endptr, 10);
    if (*endptr != '\0' || temp_height > UINT16_MAX) {
        log_e("Invalid camera height value: %s", height_value);
        return -1;
    }

    *width = (uint16_t)temp_width;
    *height = (uint16_t)temp_height;
    
    log_i("Get camera resolution: %ux%u", *width, *height);
    return 0;
}




/*设置RS485波特率*/
int Set_g_Rs485Baudrate(uint32_t Rs485Baudrate)
{
    log_i("Set RS485 baudrate: %u", Rs485Baudrate);
    
    char buffer[16];  
    snprintf(buffer, sizeof(buffer), "%u", Rs485Baudrate); 
    
    if (save_to_config("slave_device", "Rs485Baudrate", buffer) != 0) {
        log_e("Failed to save RS485 baudrate to config");
        return -1;
    }
    
    if (write_ini() != 0) {
        log_e("Failed to write config to file");
        return -1;
    }
    
    return 0;
}

int Get_g_Rs485Baudrate(uint32_t *Rs485Baudrate)
{
    if (Rs485Baudrate == NULL) {
        log_e("Invalid parameter: Rs485Baudrate is NULL");
        return -1;
    }

    const char *value = get_config_value("slave_device", "Rs485Baudrate");
    if (value == NULL || strcmp(value, "NULL") == 0) {
        log_e("RS485 baudrate value is NULL!");
        return -1;
    }

    char *endptr;
    unsigned long baudrate = strtoul(value, &endptr, 10);
    
    if (*endptr != '\0' || baudrate > UINT32_MAX) {
        log_e("Invalid RS485 baudrate value: %s", value);
        return -1;
    }

    *Rs485Baudrate = (uint32_t)baudrate;
    log_i("Get RS485 baudrate: %u", *Rs485Baudrate);
    return 0;
}