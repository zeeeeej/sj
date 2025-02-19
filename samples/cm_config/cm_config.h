#ifndef __CM_CONFIG_H__
#define __CM_CONFIG_H__
#include <stdint.h>
#include <stdio.h>

typedef struct {
    uint16_t width;
    uint16_t height;
} CameraSizeConfig;

extern CameraSizeConfig camera_size_config;


/*抓图方向*/
int Get_Gyroscope_Capture_image_direction(uint8_t *direction);
int Set_Gyroscope_Capture_image_direction(uint8_t direction);
/*陀螺仪使能*/
int Get_Gyroscope_Enable_Status(uint8_t *Enable_Status);
int Set_Gyroscope_Enable_Status(uint8_t Enable_Status);
/*从机地址*/
int Set_g_slave_address(uint8_t slave_address);
int Get_g_slave_address(uint8_t *slave_address);
/*固件版本*/
int Set_g_firmwareVersion(const char *version_str);
int Get_g_firmwareVersion(char *version_str, size_t max_len);
/*相机宽高*/
int Set_Camera_Resolution(uint16_t width, uint16_t height);
int Get_Camera_Resolution(uint16_t *width, uint16_t *height);
/*RS485波特率*/
int Set_g_Rs485Baudrate(uint32_t Rs485Baudrate);
int Get_g_Rs485Baudrate(uint32_t *Rs485Baudrate);

int Get_PWM_Frequency(uint32_t *frequency);
int Get_PWM_Duty_Cycle(uint8_t *duty_cycle);
int Get_PWM_GPIO_Pin(uint8_t *gpio_pin);


/*图片Size*/
CameraSizeConfig Get_Camera_config();
int Set_Camera_config(uint16_t width, uint16_t height);
/*图片亮度*/
int Get_Luminance();
int Set_g_Luminance(uint16_t luminance);
/*压缩率*/
int Get_compressibility();
int Set_g_compressibility(int compression);

#endif
