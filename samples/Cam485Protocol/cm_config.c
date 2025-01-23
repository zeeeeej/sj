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

void Get_g_slave_address()
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

void Get_Camera_config()
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

void LoadConfig()
{
    parse_ini();
    Get_g_slave_address();
    Get_Camera_config();
}
