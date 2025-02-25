#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "cm_config.h"
#include "elog.h"
#define MEMORY_ADDRESS_BASE 0x13540200  // 起始物理地址
#define MAX_SN_LENGTH 128               // 最大序列号长度

static void get_last_two_digits(unsigned int value, char *output, size_t output_size)
{
    if (output == NULL || output_size < 3) {
        log_e("Invalid output buffer");
        return;
    }
    
    unsigned int last_two_bytes = value & 0xFF;  // 获取最低 8 位
    snprintf(output, output_size, "%02x", last_two_bytes);
}


unsigned int read_mem(unsigned int address)
{
    unsigned int value;
    char command[128];
    snprintf(command, sizeof(command), "devmem 0x%x", address);
    FILE *fp = popen(command, "r");
    if (fp == NULL) 
    {
        perror("popen failed");
        exit(1);
    }
    // 读取并解析值
    if (fscanf(fp, "%x", &value) != 1) 
    {
        fprintf(stderr, "Failed to read memory at address 0x%x\n", address);
        pclose(fp);  // 使用 pclose 关闭管道
        exit(1);
    }
    log_i("Read memory at address 0x%x: 0x%x", address, value);
    pclose(fp);  // 使用 pclose 关闭管道
    return value;
}

// 获取摄像头序列号
int get_camera_sn(char *sn, size_t max_len)
{
    if (sn == NULL || max_len < MAX_SN_LENGTH) {
        log_e("Invalid parameters: sn buffer too small or NULL");
        return -1;
    }

    // 初始化序列号缓冲区
    memset(sn, 0, max_len);
    
    char temp[3] = {0};  // 临时缓冲区，用于存储两位十六进制数
    unsigned int value;
    size_t current_length = 0;

    // 读取前 10 个地址
    for (int i = 0; i < 10; ++i) {
        value = read_mem(MEMORY_ADDRESS_BASE + i);
        if (value == 0) {  // 检查读取是否成功
            log_e("Failed to read memory at offset %d", i);
            return -1;
        }
        
        get_last_two_digits(value, temp, sizeof(temp));
        if (current_length + 2 >= max_len) {  // 检查缓冲区大小
            log_e("SN buffer overflow");
            return -1;
        }
        
        strcat(sn, temp);
        current_length += 2;
        usleep(100);  // 短暂延迟
    }

    // 读取额外的两个特定地址
    unsigned int special_addresses[] = {0x1354020a, 0x1354020b};
    for (int i = 0; i < 2; ++i) {
        value = read_mem(special_addresses[i]);
        if (value == 0) {
            log_e("Failed to read memory at address 0x%x", special_addresses[i]);
            return -1;
        }
        
        get_last_two_digits(value, temp, sizeof(temp));
        if (current_length + 2 >= max_len) {
            log_e("SN buffer overflow");
            return -1;
        }
        
        strcat(sn, temp);
        current_length += 2;
    }

    log_i("Generated camera SN: %s", sn);
    return 0;
}

// 生成并保存序列号
int generate_sn(void)
{
    char sn[MAX_SN_LENGTH] = {0};
    
    if (get_camera_sn(sn, sizeof(sn)) != 0) {
        log_e("Failed to get camera SN");
        return -1;
    }

    // 保存到配置文件
    if (save_to_config("camera", "camera_sn", sn) != 0) {
        log_e("Failed to save camera SN to config");
        return -1;
    }

    // 写入配置文件
    if (write_ini() != 0) {
        log_e("Failed to write config to file");
        return -1;
    }

    return 0;
}