#include "cm_config.h"


void Set_g_slave_address(uint8_t slave_address)
{
    log_i("slave_address : %d\n",slave_address);
    static char buffer[16];  
    snprintf(buffer, sizeof(buffer), "%u", slave_address); 
    save_to_config("slave_device", "slave_address", buffer);
    write_ini();
    return;
}

void Set_g_firmwareVersion(uint32_t firmwareVersion)
{
    log_i("slave_address : %d\n",firmwareVersion);
    static char buffer[16];  
    snprintf(buffer, sizeof(buffer), "%u", firmwareVersion); 
    save_to_config("slave_device", "firmwareVersion", buffer);
    write_ini();
    return;
}

void Set_g_CameraNum(uint32_t CameraNum)
{
    log_i("CameraNum : %d\n",CameraNum);
    static char buffer[16];  
    snprintf(buffer, sizeof(buffer), "%u", CameraNum); 
    save_to_config("slave_device", "CameraNum", buffer);
    write_ini();
    return;
}

void Set_g_Rs485Baudrate(uint32_t Rs485Baudrate)
{
    log_i("Rs485Baudrate : %d\n",Rs485Baudrate);
    static char buffer[16];  
    snprintf(buffer, sizeof(buffer), "%u", Rs485Baudrate); 
    save_to_config("slave_device", "Rs485Baudrate", buffer);
    write_ini();
    return;
}

void Set_Camera_config(uint16_t width, uint16_t height)
{
    static char buffer[16];  
    snprintf(buffer, sizeof(buffer), "%u", width); 
    save_to_config("camera", "width", buffer);
    snprintf(buffer, sizeof(buffer), "%u", height); 
    save_to_config("camera", "height", buffer);
    write_ini();
    return;
}

void Set_Luminance(uint16_t luminance)
{
    static char buffer[16];  
    snprintf(buffer, sizeof(buffer), "%u", luminance); 
    save_to_config("camera", "luminance", buffer);
    write_ini();
    Luminance = luminance;
    return;
}

// 判断字符串是否为纯数字
int isPureDigits(const char *str) {
    while (*str!= '\0') {
        // 若字符不是数字，返回-1
        if (!isdigit(*str)) {
            return -1;
        }
        str++;
    }
    // 字符串所有字符都是数字，返回0
    return 0;
}

uint8_t Get_slave_address()
{
    uint8_t slave_address = 0;
    
    const char *value = get_config_value("slave_device", "slave_address");
    log_i("slave_address : %s\n",value);

    if(0 != isPureDigits(value))
    {
        return 0;
    }
    slave_address = atoi(value);
    return slave_address;
}

uint32_t Get_Firwareversion()
{
    uint32_t firmwareVersion = 0;
    
    const char *value = get_config_value("slave_device", "firmwareVersion");
    log_i("firmwareVersion : %s\n",value);
    if(0 != isPureDigits(value))
    {
        return 0;
    }
    firmwareVersion = atoi(value);
    return firmwareVersion;
}

uint32_t Get_CameraNum()
{
    uint32_t CameraNum = 0;
    
    const char *value = get_config_value("slave_device", "CameraNum");
    log_i("CameraNum : %s\n",value);
    if(0 != isPureDigits(value))
    {
        return 0;
    }
    CameraNum = atoi(value);
    return CameraNum;
}

uint32_t Get_Rs485Baudrate()
{
    uint32_t Rs485Baudrate = 0;
    
    const char *value = get_config_value("slave_device", "Rs485Baudrate");
    log_i("Rs485Baudrate : %s\n",value);
    if(0 != isPureDigits(value))
    {
        return 0;
    }
    Rs485Baudrate = atoi(value);
    return Rs485Baudrate;
}

const char * Get_Camera_config()
{
    const char *value = get_config_value("camera", "width");
    log_i("width : %s\n",value);
    if (value != "NULL")
    {
        uint16_t width = atoi(value);
        crop_width = width;
    }
    else
    {
        uint16_t width = 0;
    }
    value = get_config_value("camera", "height");
    log_i("height : %s\n",value);
    if (value != "NULL")
    {
        uint16_t height = atoi(value);
        crop_height = height;
    }
    else
    {
        uint16_t height = 0;
    }
}

const char *getLuminance()
{
    const char *value = get_config_value("camera", "luminance");
    log_i("luminance : %s\n",value);
    if (value != "NULL")
    {
        return value;
    }
    Luminance = atoi(value);
    return "nullptr";
}

void LoadConfig()
{
    parse_ini();
    Get_slave_address();
    Get_Camera_config();
    getLuminance();
}
