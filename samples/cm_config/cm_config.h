#ifndef __CM_CONFIG_H__
#define __CM_CONFIG_H__
#include <stdint.h>
#include <stdio.h>

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
#endif
