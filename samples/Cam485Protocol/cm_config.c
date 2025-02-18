#include "cm_config.h"

uint16_t g_slave_address = 0;

void Set_g_slave_address(uint16_t slave_address)
{
    log_i("slave_address : %d\n",slave_address);
    g_slave_address = slave_address;
    static char buffer[16];  
    snprintf(buffer, sizeof(buffer), "%u", slave_address); 
    save_to_config("slave_device", "slave_address", buffer);
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

const char * Get_g_slave_address()
{
    const char *value = get_config_value("slave_device", "slave_address");
    log_i("slave_address : %s\n",value);
    if (value != "NULL")
    {
        g_slave_address = atoi(value);
    }
    else
    {
        g_slave_address = 0;
    }
}

CameraConfig Get_Camera_config() 
{
    CameraConfig config = {0, 0};  // 初始化默认值

    const char* value = get_config_value("camera", "width");
    log_i("width : %s\n", value);
    if (value != "NULL") {
        config.width = atoi(value);  // 设置宽度
        cemare_crop_width = atoi(value);
    }

    value = get_config_value("camera", "height");
    log_i("height : %s\n", value);
    if (value != "NULL") {
        config.height = atoi(value);  // 设置高度
        cemare_crop_height = atoi(value);
    }

    return config;  // 返回包含宽度和高度的结构体
}

uint16_t Get_compressibility()
{
    const char *value = get_config_value("camera", "compressibility");
    log_i("compressibility : %s\n",value);
    if (value != "NULL")
    {
        return atoi(value);
    }
}

const char *ReadFirware()
{
    char *buffer;
    size_t buffer_size;
    FILE *file = fopen(VERSION_FILE, "r");
    if (!file) 
    {
        log_e("无法打开版本文件");
        return "";
    }
    if (fgets(buffer, buffer_size, file) == NULL) 
    {
        log_e("读取版本号失败");
        fclose(file);
        return;
    }
    buffer[strcspn(buffer, "\n")] = '\0';
    fclose(file);
    return buffer;
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
    Get_g_slave_address();
    Get_Camera_config();
    getLuminance();
}
