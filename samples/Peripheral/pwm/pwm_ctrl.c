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

// 添加PWM控制状态枚举
typedef enum {
    PWM_STATE_OFF = 0,
    PWM_STATE_ON = 1
} PWMState;

// PWM配置结构体
typedef struct {
    uint8_t gpio_pin;
    uint8_t duty_cycle;
    uint32_t frequency;
    pthread_mutex_t lock;
    pthread_t thread_id;
    volatile PWMState running_state;  // 添加运行状态标志
} PWMConfig;

// 全局PWM配置
static PWMConfig global_pwm_config;

// 修改PWM线程函数
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

    int gpio_fd = open(value_path, O_WRONLY);
    if (gpio_fd < 0) {
        log_e("Unable to open GPIO value file");
        return NULL;
    }

    char value_str[2] = {'0', '\0'};
    
    while (1) {
        pthread_mutex_lock(&config->lock);
        PWMState current_state = config->running_state;
        uint8_t current_duty = config->duty_cycle;
        int period_us = 1000000 / config->frequency;
        int high_time = period_us * current_duty / 100;
        int low_time = period_us - high_time;
        pthread_mutex_unlock(&config->lock);

        // 根据运行状态和占空比控制输出
        if (current_state == PWM_STATE_ON) {
            if (current_duty == 0) {
                // 占空比为0，直接输出低电平
                value_str[0] = '0';
                write(gpio_fd, value_str, 1);
                usleep(100000);  // 100ms检查一次状态
            }
            else if (current_duty == 100) {
                // 占空比为100，直接输出高电平
                value_str[0] = '1';
                write(gpio_fd, value_str, 1);
                usleep(100000);  // 100ms检查一次状态
            }
            else {
                // 正常PWM输出
                value_str[0] = '1';
                write(gpio_fd, value_str, 1);
                usleep(high_time);
                
                value_str[0] = '0';
                write(gpio_fd, value_str, 1);
                usleep(low_time);
            }
        } else {
            // 关闭状态，输出低电平
            value_str[0] = '0';
            write(gpio_fd, value_str, 1);
            usleep(100000);  // 100ms检查一次状态
        }
    }

    close(gpio_fd);
    return NULL;
}

// 修改设置占空比的接口，添加范围检查
void set_pwm_duty_cycle_encapsulated(int new_duty_cycle) {
    // 确保占空比在有效范围内
    if (new_duty_cycle < 0) {
        new_duty_cycle = 0;
    } else if (new_duty_cycle > 100) {
        new_duty_cycle = 100;
    }

    pthread_mutex_lock(&global_pwm_config.lock);
    global_pwm_config.duty_cycle = new_duty_cycle;
    pthread_mutex_unlock(&global_pwm_config.lock);
    log_i("Duty cycle set to %d%%", new_duty_cycle);
}

// 添加加热环开启接口
int start_heating() {
    pthread_mutex_lock(&global_pwm_config.lock);
    global_pwm_config.running_state = PWM_STATE_ON;
    pthread_mutex_unlock(&global_pwm_config.lock);
    log_i("Heating started");
    return 0;
}

// 添加加热环关闭接口
int stop_heating() {
    pthread_mutex_lock(&global_pwm_config.lock);
    global_pwm_config.running_state = PWM_STATE_OFF;
    pthread_mutex_unlock(&global_pwm_config.lock);
    log_i("Heating stopped");
    return 0;
}

// 修改初始化函数，添加初始状态
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

    // 初始化为关闭状态
    global_pwm_config.running_state = PWM_STATE_OFF;

    log_i("Configuration: GPIO_PIN = %d, DUTY_CYCLE = %d, FREQUENCY = %d, STATE = %d",
          global_pwm_config.gpio_pin,
          global_pwm_config.duty_cycle,
          global_pwm_config.frequency,
          global_pwm_config.running_state);

    pthread_mutex_init(&global_pwm_config.lock, NULL);

    if (pthread_create(&global_pwm_config.thread_id, NULL, pwm_thread, &global_pwm_config) != 0) {
        log_e("create pwm thread");
        return -1;
    }
    pthread_detach(global_pwm_config.thread_id);

    /*开启加热*/
    start_heating();
    return 0;
}

// 获取当前加热状态的接口
int get_heating_state() {
    PWMState current_state;
    pthread_mutex_lock(&global_pwm_config.lock);
    current_state = global_pwm_config.running_state;
    pthread_mutex_unlock(&global_pwm_config.lock);
    return current_state;
}

