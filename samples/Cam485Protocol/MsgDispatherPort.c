// 系统头文件
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

// 项目头文件
#include "cm_uart.h"
#include "MsgDispatherPort.h"
#include "MsgDispatcher.h"
#include "cm_common.h"
#include "circular_buffer.h"
#include "elog.h"

#define TAG_NAME  "[485]"
/*串口循环缓冲区大小*/
#define UART_CIR_BUF_SIZE   10240
static char *dev = "/dev/ttyS0";
static int uart_fd;
static int baudrate = 460800;
static pthread_mutex_t uart_mutex = PTHREAD_MUTEX_INITIALIZER;
static CircularBuffer *cb;
static int gpio_fd = -1;

// 添加错误码定义
#define UART_SUCCESS 0
#define UART_ERR_INVALID_PARAM -1
#define UART_ERR_HARDWARE -2
#define UART_ERR_TIMEOUT -3

static int init_gpio_control() {
    char value_path[64];
    snprintf(value_path, sizeof(value_path), "/sys/class/gpio/gpio53/value");
    gpio_fd = open(value_path, O_WRONLY);
    if (gpio_fd < 0) {
        log_e("Failed to open GPIO control");
        return -1;
    }
    return 0;
}

static int enable_uart_recv()
{
    if (gpio_fd < 0) return -1;
    int ret = write(gpio_fd, "0", 1);
    if (ret != 1) {
        log_e("Failed to set GPIO for recv, ret=%d", ret);
        return -1;
    }
    // 添加小延时确保GPIO状态稳定
    usleep(100);
    return 0;
}

static int enable_uart_send()
{
    if (gpio_fd < 0) return -1;
    int ret = write(gpio_fd, "1", 1);
    if (ret != 1) {
        log_e("Failed to set GPIO for send, ret=%d", ret);
        return -1;
    }
    // 添加小延时确保GPIO状态稳定
    usleep(100);
    return 0;
}

static int uart_ready()
{
    if (access("/sys/class/gpio/gpio53", F_OK) != 0)
    {
        LOGD("init serial port gpio53\n");
        char buf[1024] = {0};
        snprintf(buf, sizeof(buf), "echo 53 > /sys/class/gpio/export");
        system(buf);
        snprintf(buf, sizeof(buf), "echo out > /sys/class/gpio/gpio53/direction");
        system(buf);
    }
    return 0;
}


static void *debug_info_thread(void *arg)
{
    int fd; // 假设这是你已经打开的串口文件描述符
    int bytes;

    while(1)
    {
        if (ioctl(uart_fd, TIOCINQ, &bytes) == 0)
            printf("Bytes in input buffer: %d\n", bytes);
        else
            perror("ioctl(TIOCINQ)");

        if (ioctl(uart_fd, TIOCOUTQ, &bytes) == 0)
            printf("Bytes in output buffer: %d\n", bytes);
        else
            perror("ioctl(TIOCOUTQ)");

        sleep(1);
    }


    return 0;
}








static int uart_recv(unsigned char *data, int len)
{
    // 加锁
    pthread_mutex_lock(&uart_mutex);

    int ret = enable_uart_recv();
    if (ret != 0)
    {
        // 解锁并返回错误
        pthread_mutex_unlock(&uart_mutex);
        return ret;
    }

    ret = cm_uart_recv_simple(uart_fd, data, len);

    // 解锁
    pthread_mutex_unlock(&uart_mutex);

    return ret;
}



/*数据接收线程*/
static void *data_recv_thread(void *arg)
{
    uint8_t data[512] = {0};
    int len = sizeof(data);
    int recv_len;
    while(1)
    {
        recv_len = uart_recv(data,len);
        if(recv_len >= 0)
        {
            size_t written = circular_buffer_write(cb, data, recv_len, -1);
            log_d("Producer wrote %zu bytes\n", written);
        }
        
        // elog_hexdump("push uart data", 16, data, recv_len);
        // push(&cb,data,recv_len);
        
        // print_buffer_contents(&cb);
        usleep(10000);
    }
}


static int uart_init()
{
    // 1. 首先初始化GPIO
    if(uart_ready() != 0) {
        log_e("uart ready failed");
        return -1;
    }

    // 2. 初始化GPIO控制
    if (init_gpio_control() != 0) {
        log_e("Failed to init GPIO control");
        return -1;
    }

    // 3. 设置接收模式
    if(enable_uart_recv() != 0) {
        log_e("enable uart recv failed");
        return -1;
    }

    // 4. 打开并初始化串口
    int ret = cm_uart_open(dev);
    if (ret > 0) {
        uart_fd = ret;
        ret = cm_uart_init(ret, baudrate, 0, 8, 1, 'n');
        if (ret < 0) {
            log_e("uart init failed");
            cm_uart_close(uart_fd);
            return -1;
        }

        log_i("uart init success dev [%s]", dev);

        // 5. 初始化循环缓冲区
        cb = circular_buffer_create(UART_CIR_BUF_SIZE);
        if(cb == NULL) {
            log_e("create uart cir buf fail");
            return -1;
        }

        // 6. 创建接收线程
        pthread_t data_recv_pid;
        if (pthread_create(&data_recv_pid, NULL, data_recv_thread, NULL) != 0) {
            log_e("Failed to create recv thread");
            return -1;
        }

        return 0;
    }

    log_e("open uart [%s] failed ret [%d]\n", dev, ret);
    return -1;
}


static int close_uart()
{
    if (uart_fd > 0)
    {
        cm_uart_close(uart_fd);
        uart_fd = -1;
    }
    if (gpio_fd > 0) {
        close(gpio_fd);
        gpio_fd = -1;
    }
    return 0;
}



static int uart_send(unsigned char *data, uint32_t len)
{
    pthread_mutex_lock(&uart_mutex);

    int ret = enable_uart_send();
    if (ret != 0) {
        log_e("enable_uart_send error");
        pthread_mutex_unlock(&uart_mutex);
        return ret;
    }

    ret = cm_uart_send_until(uart_fd, data, len);
    
    // 发送完成后切换回接收模式
    enable_uart_recv();
    
    pthread_mutex_unlock(&uart_mutex);
    return ret;
}



static int uart_read(unsigned char *data, uint32_t len, int time_out_ms)
{
    if (!data || len == 0) {
        log_e("Invalid parameters");
        return -1;
    }

    size_t read_bytes = circular_buffer_read_exact(cb, data, len, time_out_ms);
    log_d("pop data length: %zu, expect len: %d", read_bytes, len);
    
    return (int)read_bytes;  // 返回实际读取的字节数
}

static int uart_set_baudrate(uint32_t baudrate)
{

    
    // 加锁保护串口操作
    pthread_mutex_lock(&uart_mutex);
    
    int ret = try_set_baudrate(uart_fd, baudrate);
    if(ret != 0)
    {
        log_e("set baudrate failed");
        pthread_mutex_unlock(&uart_mutex);
        return ret;
    }
    log_i("Setting UART baudrate to: %u", baudrate);
    pthread_mutex_unlock(&uart_mutex);
    return ret;
}



static int uart_control(int control_code, void *user_data, uint32_t len)
{
    if (user_data == NULL || len == 0) {
        log_e("Invalid control parameters");
        return UART_ERR_INVALID_PARAM;
    }

    switch (control_code) {
        case UART_CTRL_SET_BAUDRATE:
            if (len != sizeof(uint32_t)) {
                log_e("Invalid baudrate parameter size: %u", len);
                return UART_ERR_INVALID_PARAM;
            }
            uint32_t baudrate = *(uint32_t*)user_data;
            return uart_set_baudrate(baudrate);
            
        default:
            log_e("Unsupported control code: %d", control_code);
            return UART_ERR_INVALID_PARAM;
    }
}

DataTransInterface uart_interface = 
{
    .init = uart_init,
    .recv_data = uart_read,
    .send_data = uart_send,
    .control = uart_control
};

// 添加接口获取函数
DataTransInterface* get_uart_interface(void)
{
    return &uart_interface;
}