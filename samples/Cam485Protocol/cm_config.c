#include "cm_config.h"
#define LOG_TAG "[CM_CONFIG]"
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


/*===================陀螺仪属性设置=======================*/
/*设置抓图方向*/
void Set_Gyroscope_Capture_image_direction(uint8_t direction)
{
    log_i("Set Gyroscope_Capture_image_direction : %d",direction);
    static char buffer[16];  
    snprintf(buffer, sizeof(buffer), "%u", direction); 
    save_to_config("Gyroscope", "CaptureDirection", direction);
    write_ini();
    return;
}
/*获取抓图方向*/
uint8_t Get_Gyroscope_Capture_image_direction()
{
    uint8_t direction = 0 ;
    const char *value = get_config_value("Gyroscope", "CaptureDirection");
    if (value != "NULL")
    {
        direction = atoi(value);
        log_i("Get Gyroscope Capture image direction : %d",direction);
    }
    else
    {
        log_e("CaptureDirection value is NULL !");
        direction = -1;
    }
    return direction;
}

/*设置陀螺仪开启状态*/
void Set_Gyroscope_Enable_Status(uint8_t Enable_Status)
{
    log_i("Set Gyroscope Enable Status : %d",Enable_Status);
    static char buffer[16];  
    snprintf(buffer, sizeof(buffer), "%u", Enable_Status); 
    save_to_config("Gyroscope", "EnableStatus", buffer);
    write_ini();
    /*关闭陀螺仪*/
    if(Enable_Status == 0)
    {
        /*关闭状态*/
        /*todo : 要检测重复开启关闭*/
        door_detect_deinit();
    }
    else
    {
        /*开启状态*/
        door_detect_init();
    }
    return;
}
/*获取陀螺仪开启状态*/
uint8_t Get_Gyroscope_Enable_Status(uint8_t *Enable_Status)
{
    if (Enable_Status == NULL) {
        log_e("Invalid parameter: Enable_Status is NULL");
        return -1;
    }

    const char *value = get_config_value("Gyroscope", "EnableStatus");
    if (strcmp(value, "NULL") != 0) {
        *Enable_Status = (uint8_t)atoi(value);
        log_i("Get Gyroscope Capture Enable Status: %d", *Enable_Status);
        return 0;
    } else {
        log_e("EnableStatus value is NULL!");
        return -1;
    }
}

/*设置陀螺仪抓图最多存储数量*/
void Set_Gyroscope_Image_Save_Count(uint8_t SaveCount)
{
    log_i("Set Gyroscope Capture image save count : %d",SaveCount);
    static char buffer[16];  
    snprintf(buffer, sizeof(buffer), "%u", SaveCount); 
    save_to_config("Gyroscope", "SaveCount", buffer);
    write_ini();
    return;
}
/*获取陀螺仪抓图最多存储数量*/
uint8_t Get_Gyroscope_Image_Save_Count(uint8_t *SaveCount)
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

const char *ReadFirware()
{
    // char *buffer;
    // size_t buffer_size;
    // FILE *file = fopen(VERSION_FILE, "r");
    // if (!file) 
    // {
    //     log_e("无法打开版本文件");
    //     return "";
    // }
    // if (fgets(buffer, buffer_size, file) == NULL) 
    // {
    //     log_e("读取版本号失败");
    //     fclose(file);
    //     return;
    // }
    // buffer[strcspn(buffer, "\n")] = '\0';
    // fclose(file);
    // return buffer;
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
