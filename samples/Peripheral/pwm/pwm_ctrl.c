#define LOG_TAG "[PWM_CTRL]"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>           // 用于 O_WRONLY 等文件操作标志
#include <sys/types.h>       // 用于基本系统数据类型
#include <sys/stat.h>        // 用于文件状态相关定义
#include "elog.h"
#include "cm_config.h"
#define CONFIG_FILE "/system/etc/cm_config.ini"

// PWM配置结构体
typedef struct {
    uint8_t gpio_pin;
    uint8_t duty_cycle;
    uint32_t frequency;
    pthread_mutex_t lock;
    pthread_t thread_id;
} PWMConfig;

// 全局PWM配置
static PWMConfig global_pwm_config;

// 从ini文件获取配置值的函数
// static char* get_config_value(const char* section, const char* key) {
//     static char value[256];
//     char line[256];
//     FILE* file = fopen(CONFIG_FILE, "r");
//     if (!file) {
//         perror("Unable to open config file");
//         return NULL;
//     }

//     int in_section = 0;
//     while (fgets(line, sizeof(line), file)) {
//         if (line[0] == '[') {
//             in_section = (strncmp(line, section, strlen(section)) == 0);
//         } else if (in_section && strstr(line, key)) {
//             char* pos = strchr(line, '=');
//             if (pos) {
//                 strcpy(value, pos + 1);
//                 value[strcspn(value, "\n")] = '\0'; // 去除换行符
//                 fclose(file);
//                 return value;
//             }
//         }
//     }
//     fclose(file);
//     return NULL;
// }

// 模拟PWM的线程函数
void* pwm_thread(void* arg) {
    PWMConfig* config = (PWMConfig*)arg;
    char command[128];

    // 导出GPIO
    snprintf(command, sizeof(command), "echo %d > /sys/class/gpio/export", config->gpio_pin);
    system(command);

    // 设置GPIO方向为输出
    snprintf(command, sizeof(command), "echo out > /sys/class/gpio/gpio%d/direction", config->gpio_pin);
    system(command);

    char value_path[64];
    snprintf(value_path, sizeof(value_path), "/sys/class/gpio/gpio%d/value", config->gpio_pin);

    // 打开文件并保持打开状态
    int gpio_fd = open(value_path, O_WRONLY);
    if (gpio_fd < 0) {
        log_e("Unable to open GPIO value file");
        return NULL;
    }

    char value_str[2] = {'0', '\0'};
    
    // 模拟PWM
    while (1) {
        pthread_mutex_lock(&config->lock);
        int period_us = 1000000 / config->frequency;
        int high_time = period_us * config->duty_cycle / 100;
        int low_time = period_us - high_time;
        pthread_mutex_unlock(&config->lock);

        // 使用write替代fprintf，减少文件系统开销
        value_str[0] = '1';
        write(gpio_fd, value_str, 1);
        printf("1\n");
        usleep(high_time*10);
        
        value_str[0] = '0';
        write(gpio_fd, value_str, 1);
        printf("0\n");

        usleep(low_time*10);
    }

    close(gpio_fd);
    return NULL;
}

// 设置PWM占空比的接口
void set_pwm_duty_cycle_encapsulated(int new_duty_cycle) {
    pthread_mutex_lock(&global_pwm_config.lock);
    global_pwm_config.duty_cycle = new_duty_cycle;
    pthread_mutex_unlock(&global_pwm_config.lock);
    log_i("Duty cycle set to %d", new_duty_cycle);
}

// 单一接口函数，用于初始化和启动PWM
int run_pwm() {
    if (Get_PWM_GPIO_Pin(&global_pwm_config.gpio_pin) != 0) {
        global_pwm_config.gpio_pin = 49; // 默认GPIO49
    }

    if (Get_PWM_Duty_Cycle(&global_pwm_config.duty_cycle) != 0) {
        global_pwm_config.duty_cycle = 30; // 默认30%
    }

    if (Get_PWM_Frequency(&global_pwm_config.frequency) != 0) {
        global_pwm_config.frequency = 1; // 默认1Hz
    }

    // 使用原有日志接口
    log_i("Configuration: GPIO_PIN = %d, DUTY_CYCLE = %d, FREQUENCY = %d",
          global_pwm_config.gpio_pin,
          global_pwm_config.duty_cycle,
          global_pwm_config.frequency);

    pthread_mutex_init(&global_pwm_config.lock, NULL);

    if (pthread_create(&global_pwm_config.thread_id, NULL, pwm_thread, &global_pwm_config) != 0) {
        log_e("create pwm thread");
        return -1;
    }
    pthread_detach(global_pwm_config.thread_id); // 分离线程
    return 0;
}

// int main() {
//     if (run_pwm() != 0) {
//         log_e("Failed to start PWM");
//         return EXIT_FAILURE;
//     }

//     // 示例：5秒后改变占空比
//     sleep(5);
//     set_pwm_duty_cycle_encapsulated(75); // 设置新的占空比为75%

//     // 主线程继续运行，线程资源自动回收
//     while (1) {
//         sleep(1); // 主线程保持运行
//     }

//     return 0;
// }