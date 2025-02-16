#include "cm_uart.h"
#include "MsgDispatherPort.h"
#include "MsgDispatcher.h"
#include "cm_common.h"
#include "circular_buffer.h"
#include "elog.h"


#include "stdio.h"
#include "stdint.h"
#include "unistd.h"
#include "sys/wait.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "errno.h"
#include "stdlib.h"
#include "termios.h"
#include <pthread.h>
#include <sys/ioctl.h>

#define TAG_NAME  "[485]"
#define UART_CIR_BUF_SIZE   10240
static char *dev = "/dev/ttyS0";
static int uart_fd;
static int baudrate = 460800;
static pthread_mutex_t uart_mutex = PTHREAD_MUTEX_INITIALIZER;
static CircularBuffer *cb;



// 添加错误码定义
#define UART_SUCCESS 0
#define UART_ERR_INVALID_PARAM -1
#define UART_ERR_HARDWARE -2
#define UART_ERR_TIMEOUT -3



static int enable_uart_recv()
{
    int ret = system("echo 0 > /sys/class/gpio/gpio53/value");
    if (ret == -1)
    {
        perror("system call failed");
        return -1;
    }
    else if (WIFEXITED(ret) && WEXITSTATUS(ret) == 0)
    {
        return 0; // Success
    }
    else
    {
        fprintf(stderr, "Command failed with exit status %d\n", WEXITSTATUS(ret));
        return -1;
    }
}

static int enable_uart_send()
{
    int ret = system("echo 1 > /sys/class/gpio/gpio53/value");
    if (ret == -1)
    {
        perror("system call failed");
        return -1;
    }
    else if (WIFEXITED(ret) && WEXITSTATUS(ret) == 0)
    {
        return 0; // Success
    }
    else
    {
        fprintf(stderr, "Command failed with exit status %d\n", WEXITSTATUS(ret));
        return -1;
    }
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
    if(uart_ready() != 0)
    {
        log_e("uart ready failed");
        return -1;
    }
    if(enable_uart_recv() != 0)
    {
        log_e("enable uart recv failed");
        return -1;
    }
    int ret = cm_uart_open(dev);
    if (ret > 0)
    {
        uart_fd = ret;
        ret = cm_uart_init(ret, baudrate, 0, 8, 1, 'n');
        if (ret < 0)
        {
            log_e("uart init failed");
            cm_uart_close(uart_fd);
        }
        else
        {
            log_i("uart init success dev [%s]", dev);

            /*初始化循环缓冲区*/
            cb = circular_buffer_create(UART_CIR_BUF_SIZE);
            if(cb==NULL)
            {
                log_e("create uart cir buf fail");
                return -1;
            }
            pthread_t data_recv_pid;
            /*建立接收485数据线程*/
            pthread_create(&data_recv_pid, NULL, data_recv_thread, NULL);

            return 0;
        }
    }
    else
    {
        log_e("open uart [%s] failed ret [%d]\n", dev, ret);
    }

    return -1;
}


static int close_uart()
{
    if (uart_fd > 0)
    {
        cm_uart_close(uart_fd);
        uart_fd = -1;
    }
    return 0;
}



static int uart_send(unsigned char *data, uint32_t len)
{
    // 加锁
    pthread_mutex_lock(&uart_mutex);

    int ret = enable_uart_send();
    if (ret != 0)
    {
        // 解锁并返回错误
        LOGD("enable_uart_send error");
        pthread_mutex_unlock(&uart_mutex);
        return ret;
    }

    ret = cm_uart_send_until(uart_fd, data, len);

    // // 刷新发送缓冲区
    // if (ret >= 0) {
    //     // TCIOFLUSH 清除输入和输出队列
    //     // TCOFLUSH 只清除输出队列
    //     tcflush(uart_fd, TCOFLUSH); // 仅刷新发送缓冲区
    //     // 或者使用 tcflush(uart_fd, TCIOFLUSH); // 清除输入和输出队列
    // }
    // 解锁
    pthread_mutex_unlock(&uart_mutex);

    return ret;
}



static int uart_read(unsigned char *data, uint32_t len)
{
    int ret =  0;
    size_t read = circular_buffer_read(cb, data, len, -1);
    log_d("pop data length : %d , expect len : %d",read,len);
    // elog_hexdump("pop uart data", 16, data, len);
    // print_buffer_contents(&cb);
    if(ret != 0)
    {
        return -1;  
    }
    return len;
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