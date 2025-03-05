#include <errno.h>   
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/serial.h>
#include <termios.h>

#include "elog.h"
#include "AttributeTable.h"
#include "MsgDispatcher.h"  
#include "cm_uart.h"
#include "MsgDispatherPort.h"
#include "cm_config.h"

// Constants
#define TAG_NAME "[ID04_485Baudrate]"
#define BAUDRATE_CHANGE_TIMEOUT_MS 5000  // 5秒超时
#define THREAD_START_TIMEOUT_MS 100      // 100ms线程启动超时

// 支持的波特率列表
static const uint32_t supported_baudrates[] = {
    9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600
};

// Type definitions
typedef struct {
    uint32_t baudrate;
    bool completed;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} BaudrateChangeParams;

typedef struct {
    uint32_t old_baudrate;
    uint32_t new_baudrate;
    bool in_progress;
    time_t start_time;
} BaudrateChangeStatus;

// Global variables
static BaudrateChangeStatus baudrate_status = {0};

// Function declarations
static bool is_baudrate_supported(uint32_t baudrate);
static int reinit_RS485(uint32_t new_baudrate);
void *timer_callback(void *args);
int RS485_reinit(uint32_t Rs485Baudrate);
int attribute_baudrate_set(const uint8_t* value, uint32_t value_len);
void initRs485(uint32_t newBaudrate);

// Helper functions
static bool is_baudrate_supported(uint32_t baudrate) {
    for (size_t i = 0; i < sizeof(supported_baudrates) / sizeof(supported_baudrates[0]); i++) {
        if (baudrate == supported_baudrates[i]) {
            return true;
        }
    }
    return false;
}

// Core functionality
static int reinit_RS485(uint32_t new_baudrate) {
    log_i("Reinitializing RS485 with baudrate: %u", new_baudrate);
    
    // 保存新的波特率设置到配置文件
    int result = Set_g_Rs485Baudrate(new_baudrate);
    if (result != 0) {
        log_e("Failed to save baudrate setting");
        return -1;
    }

    // 获取UART接口
    DataTransInterface* uart_if = get_uart_interface();
    if (uart_if == NULL) {
        log_e("Failed to get UART interface");
        return -1;
    }

    // 通过控制接口设置新波特率
    result = uart_if->control(UART_CTRL_SET_BAUDRATE, &new_baudrate, sizeof(new_baudrate));
    if (result != 0) {
        log_e("Failed to set new baudrate");
        return -1;
    }

    log_i("RS485 reinitialized successfully");
    return 0;
}


// static int get_cur_baudrate()
// {
//     DataTransInterface* uart_if = get_uart_interface();
//     if (uart_if == NULL) {
//         log_e("Failed to get UART interface");
//         return -1;
//     }
//     uint32_t baudrate;
//     uart_if->control(UART_CTRL_GET_BAUDRATE, &baudrate, sizeof(baudrate));
//     return baudrate;
// }

void *timer_callback(void *args) {
    BaudrateChangeParams *params = (BaudrateChangeParams *)args;
    if (params == NULL) {
        log_e("Invalid timer callback parameters");
        return NULL;
    }

    log_i("Timer callback executing with baudrate: %u", params->baudrate);
    
    // 延时2秒等待之前的通信完成
    usleep(2000000);  // 2 seconds
    
    // 重新初始化RS485
    int result = reinit_RS485(params->baudrate);
    
    // 发送设置结果响应
    // SendSetAttributeResp(SID03_Attribute_Baudrate, (result == 0) ? 0 : 1);

    // 通知完成状态
    pthread_mutex_lock(&params->mutex);
    params->completed = true;
    pthread_cond_signal(&params->cond);
    pthread_mutex_unlock(&params->mutex);
    
    return NULL;
}

int RS485_reinit(uint32_t Rs485Baudrate) {
    log_i("Starting RS485 reinit with baudrate: %u", Rs485Baudrate);
    
    // 创建并初始化参数结构
    BaudrateChangeParams *params = calloc(1, sizeof(BaudrateChangeParams));
    if (params == NULL) {
        log_e("Failed to allocate memory for thread parameters");
        return -1;
    }
    
    params->baudrate = Rs485Baudrate;
    params->completed = false;
    pthread_mutex_init(&params->mutex, NULL);
    pthread_cond_init(&params->cond, NULL);

    // 创建定时器线程
    pthread_t timerThread;
    if (pthread_create(&timerThread, NULL, timer_callback, params) != 0) {
        log_e("Failed to create timer thread");
        free(params);
        return -1;
    }
    
    pthread_detach(timerThread);
    log_i("Timer thread started");

    // 等待完成或超时
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += BAUDRATE_CHANGE_TIMEOUT_MS / 1000;
    ts.tv_nsec += (BAUDRATE_CHANGE_TIMEOUT_MS % 1000) * 1000000;

    pthread_mutex_lock(&params->mutex);
    int result = 0;
    while (!params->completed) {
        result = pthread_cond_timedwait(&params->cond, &params->mutex, &ts);
        if (result == ETIMEDOUT) {
            log_e("Baudrate change timeout");
            break;
        }
    }
    pthread_mutex_unlock(&params->mutex);

    // 清理资源
    pthread_mutex_destroy(&params->mutex);
    pthread_cond_destroy(&params->cond);
    free(params);

    return (result == 0) ? 0 : -1;
}


int attribute_baudrate_set(const uint8_t* value, uint32_t value_len) {
    // if (value == NULL || value_len < 4) {
    //     log_e("Invalid baudrate parameters");
    //     return -1;
    // }

    uint32_t new_baudrate = ((value[0])|(value[1]<<8)|(value[2]<<16)|(value[3]<<24));
    log_i("Setting baudrate to: %u", new_baudrate);
    
    // 验证波特率值是否支持
    if (!is_baudrate_supported(new_baudrate)) {
        log_e("Unsupported baudrate value: %u", new_baudrate);
        SendSetAttributeResp(0x04, 1);  // 发送失败响应
        return -1;
    }

    // 获取当前波特率
    DataTransInterface* uart_if = get_uart_interface();
    uint32_t old_baudrate;
    int ret = uart_if->control(UART_CTRL_GET_BAUDRATE, &old_baudrate, sizeof(old_baudrate));
    if (ret != 0) {
        log_e("Failed to get current baudrate");
        SendSetAttributeResp(0x04, 1);
        return -1;
    }
    
    log_i("Current baudrate: %u, attempting to set new baudrate: %u", old_baudrate, new_baudrate);

    // 尝试设置新波特率
    ret = reinit_RS485(new_baudrate);
    if (ret != 0) {
        log_e("Failed to set new baudrate");
        SendSetAttributeResp(0x04, 1);
        return -1;
    }

    // 等待一小段时间确保新波特率生效
    usleep(100000);  // 100ms

    // 切回旧波特率发送响应
    ret = reinit_RS485(old_baudrate);
    if (ret != 0) {
        log_e("Failed to restore old baudrate");
        return -1;
    }

    // 发送成功响应
    SendSetAttributeResp(0x04, 0);

    // 启动异步线程进行最终的波特率切换
    RS485_reinit(new_baudrate);

    return 0;
}

int attribute_baudrate_get()
{

    return 0;
}