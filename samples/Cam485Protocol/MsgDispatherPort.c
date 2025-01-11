
#include "cm_uart.h"
#include "MessageDispaterPort.h"
#include "MsgDispatcher.h"
#include "cm_common.h"

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
static char *dev = "/dev/ttyS0";
static int uart_fd;

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
static int uart_init()
{
    if(uart_ready() != 0)
    {
        LOGD("uart ready failed\n");
        return -1;
    }
    if(enable_uart_recv() != 0)
    {
        LOGD("enable uart recv failed\n");
        return -1;
    }
    int ret = cm_uart_open(dev);
    if (ret > 0)
    {
        uart_fd = ret;
        ret = cm_uart_init(ret, 230400, 0, 8, 1, 'n');
        if (ret < 0)
        {
            LOGD("uart init failed\n");
            cm_uart_close(uart_fd);
        }
        else
        {
            LOGD("uart init success dev [%s]", dev);
            return 0;
        }
    }
    else
    {
        LOGD("ERROR: open uart [%s] failed ret [%d]\n", dev, ret);
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


static int uart_recv(unsigned char *data, int len)
{
    enable_uart_recv();
    return cm_uart_recv_simple(uart_fd, data, len);
}

static int uart_send(unsigned char *data, int len)
{
    enable_uart_send();
    return cm_uart_send_until(uart_fd, data, len);
}





DataTransInterface uart_interface = 
{
    .init = uart_init,
    .recv_data = uart_recv,
    .send_data = uart_send
};