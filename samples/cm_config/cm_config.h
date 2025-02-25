#ifndef __CM_CONFIG_H__
#define __CM_CONFIG_H__
#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>
#include "ParseIni.h"




#define NUM_READS 12    // 总共读取的地址数

#define SN_FILE_PATH "/tmp/chip_id.txt"

/*抓图方向*/
int Get_Gyroscope_Capture_image_direction(uint8_t *direction);
int Set_Gyroscope_Capture_image_direction(uint8_t direction);
/*陀螺仪使能*/
int Get_Gyroscope_Enable_Status(uint8_t *Enable_Status);
int Set_Gyroscope_Enable_Status(uint8_t Enable_Status);
/*陀螺仪抓图角度*/
int Get_Gyroscope_Capture_image_angle(uint8_t *angleA, uint8_t *angleB);
int Set_Gyroscope_Capture_image_angle(uint8_t angleA, uint8_t angleB);
/*陀螺仪抓图最多存储数量*/
int Set_Gyroscope_Image_Save_Count(uint8_t SaveCount);
int Get_Gyroscope_Image_Save_Count(uint8_t *SaveCount);
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
/*pwm*/
int Get_PWM_Frequency(uint32_t *frequency);
int Get_PWM_Duty_Cycle(uint8_t *duty_cycle);
int Get_PWM_GPIO_Pin(uint8_t *gpio_pin);


/*图片Size*/
int Get_Camera_config(uint16_t *width, uint16_t *height);
int Set_Camera_config(uint16_t width, uint16_t height);
/*图片亮度*/
int Get_Luminance(uint8_t *luminance);
int Set_g_Luminance(uint8_t luminance);
/*压缩率*/
int Get_compressibility(uint8_t *compressibility);
int Set_g_compressibility(uint8_t compression);

/*摄像头sn*/
int  Get_g_CameraSn(char *sn, uint8_t max_len);

#endif
