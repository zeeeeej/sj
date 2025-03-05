//
// Created by v on 18-6-21.
//

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include "cm_uart.h"
static speed_t get_baud_constant(int baudrate) {
    switch(baudrate) {
        case 9600:    return B9600;
        case 19200:   return B19200;
        case 38400:   return B38400;
        case 57600:   return B57600;
        case 115200:  return B115200;
        case 230400:  return B230400;
        case 460800:  return B460800;
        case 921600:  return B921600;
        default:      
            printf("Unsupported baudrate: %d\n", baudrate);
            return B0;  // 表示无效波特率
    }
}

int try_set_baudrate(int fd, int baudrate) {  // 注意这里改为 int baudrate
    struct termios options;
    
    // 检查文件描述符是否有效
    if (fd < 0) {
        printf("Invalid file descriptor: %d\n", fd);
        return -1;
    }

    printf("Requested baudrate: %d\n", baudrate);
    
    // 将实际波特率转换为系统常量
    speed_t baud_const = get_baud_constant(baudrate);
    if (baud_const == B0) {
        return -1;
    }
    
    // 获取当前串口配置
    if (tcgetattr(fd, &options) != 0) {
        printf("Failed to get serial port attributes: %s (errno: %d)\n", strerror(errno), errno);
        return -1;
    }

    // 设置输入波特率
    if (cfsetispeed(&options, baud_const) != 0) {
        printf("Failed to set input baudrate: %s (errno: %d)\n", strerror(errno), errno);
        return -1;
    }

    // 设置输出波特率
    if (cfsetospeed(&options, baud_const) != 0) {
        printf("Failed to set output baudrate: %s (errno: %d)\n", strerror(errno), errno);
        return -1;
    }

    // 应用新的串口配置
    if (tcsetattr(fd, TCSANOW, &options) != 0) {
        printf("Failed to set serial port attributes: %s (errno: %d)\n", strerror(errno), errno);
        return -1;
    }

    printf("Successfully set baudrate to %d\n", baudrate);
    return 0;
}

/*获取当前串口波特率*/
int cm_uart_get_baudrate(int fd, int *baudrate)
{
    struct termios options;
    speed_t speed;

    if (fd < 0 || baudrate == NULL) {
        return -1;
    }

    // 获取当前串口配置
    if (tcgetattr(fd, &options) != 0) {
        printf("Failed to get serial port attributes: %s (errno: %d)\n", 
               strerror(errno), errno);
        return -2;
    }

    // 获取输入波特率（通常输入输出波特率是一致的）
    speed = cfgetispeed(&options);

    // 将系统常量转换为实际波特率值
    switch(speed) {
        case B9600:    *baudrate = 9600;    break;
        case B19200:   *baudrate = 19200;   break;
        case B38400:   *baudrate = 38400;   break;
        case B57600:   *baudrate = 57600;   break;
        case B115200:  *baudrate = 115200;  break;
        case B230400:  *baudrate = 230400;  break;
        case B460800:  *baudrate = 460800;  break;
        case B921600:  *baudrate = 921600;  break;
        default:
            printf("Unknown baudrate constant: %d\n", (int)speed);
            return -3;
    }

    return 0;
}

int cm_uart_open(char *serial_port)
{
    int fd;

    fd = open(serial_port, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd < 0)
    {
        return -1;
    }

    if (fcntl(fd, F_SETFL, 0) < 0)
    {
        close(fd);
        return -2;
    }
    // for (size_t i = 0; i < sizeof(baudrates) / sizeof(baudrates[0]); i++) {
    //     if (try_set_baudrate(fd, baudrates[i]) == 0) {
    //         printf("Baudrate %ld supported\n", (long)baudrates[i]);
    //     } else {
    //         printf("Baudrate %ld not supported\n", (long)baudrates[i]);
    //     }
    // }
    return fd;
}

void cm_uart_close(int fd) { close(fd); }

int cm_uart_init(int fd, int speed, int flow_ctrl, int databits, int stopbits,
                 int parity)
{

    int i;
    int speed_arr[] = {B460800, B1152000, B230400, B921600, B576000, B115200, B19200, B9600, B4800, B2400, B1200, B300};
    int name_arr[]  = {460800, 1152000, 230400, 921600, 576000, 115200, 19200, 9600, 4800, 2400, 1200, 300};

    struct termios options;

    if (tcgetattr(fd, &options) != 0)
    {
        return (-1);
    }

    for (i = 0; i < (int)(sizeof(speed_arr) / sizeof(int)); i++)
    {
        if (speed == name_arr[i])
        {
            int ret = cfsetispeed(&options, speed_arr[i]);
            printf("cfsetispeed:%d, errno [%d]\n", ret, errno);
            ret = cfsetospeed(&options, speed_arr[i]);
            printf("cfsetospeed:%d, errno [%d]\n", ret, errno);
            break;
        }
    }

    if (i == sizeof(speed_arr) / sizeof(int))
    {
        return -2;
    }

    options.c_cflag |= CLOCAL;
    options.c_cflag |= CREAD;

    switch (flow_ctrl)
    {
    case 0:
        options.c_cflag &= ~CRTSCTS;
        break;
    case 1:
        options.c_cflag |= CRTSCTS;
        break;
    case 2:
        options.c_cflag |= IXON | IXOFF | IXANY;
        break;
    }

    options.c_cflag &= ~CSIZE;
    switch (databits)
    {
    case 5:
        options.c_cflag |= CS5;
        break;
    case 6:
        options.c_cflag |= CS6;
        break;
    case 7:
        options.c_cflag |= CS7;
        break;
    case 8:
        options.c_cflag |= CS8;
        break;
    default:
        return (-3);
    }

    switch (parity)
    {
    case 'n':
    case 'N':
        options.c_cflag &= ~PARENB;
        options.c_iflag &= ~INPCK;
        break;
    case 'o':
    case 'O':
        options.c_cflag |= (PARODD | PARENB);
        options.c_iflag |= INPCK;
        break;
    case 'e':
    case 'E':
        options.c_cflag |= PARENB;
        options.c_cflag &= ~PARODD;
        options.c_iflag |= INPCK;
        break;
    case 's':
    case 'S':
        options.c_cflag &= ~PARENB;
        options.c_cflag &= ~CSTOPB;
        break;
    default:
        return (-4);
    }

    switch (stopbits)
    {
    case 1:
        options.c_cflag &= ~CSTOPB;
        break;
    case 2:
        options.c_cflag |= CSTOPB;
        break;
    default:
        return (-5);
    }

    options.c_oflag &= ~OPOST;
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    // options.c_lflag &= ~(ISIG | ICANON);
    options.c_iflag &= ~(ICRNL | IGNCR | IXON);

    options.c_cc[VTIME] = 1;
    options.c_cc[VMIN]  = 0; // modify by dengxingsheng@20171010: 1 -> 0

    tcflush(fd, TCIFLUSH);

    if (tcsetattr(fd, TCSANOW, &options) != 0)
    {
        return (-6);
    }

    return (0);
}

int cm_uart_recv(int fd, unsigned char *rcv_buf, int data_len, int sec,
                 int usec)
{
    int            count = 0;
    int            len = 0, ret = 0;
    fd_set         fs_read;
    struct timeval timeout;

    timeout.tv_sec  = sec;
    timeout.tv_usec = usec;
    while (count < data_len)
    {
        FD_ZERO(&fs_read);
        FD_SET(fd, &fs_read);

        ret = select(fd + 1, &fs_read, NULL, NULL, &timeout);
        if (ret == -1)
        {
            break;
        }
        else if (ret)
        {
            len = read(fd, rcv_buf + count, data_len - count);
            count += len;
        }
        else
        {
            break;
        }
    }

    return count;
}

int cm_uart_recv_simple(int fd, unsigned char *rcv_buf, int data_len)
{
    int ret = read(fd, rcv_buf, data_len);
    if (ret < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            return 0;
        }
    }
    return ret;
}

int cm_uart_send(int fd, unsigned char *send_buf, int data_len)
{
    int len = 0;

    len = write(fd, send_buf, data_len);
    if (len == data_len)
    {
        return len;
    }
    else
    {
        tcflush(fd, TCOFLUSH);
        return -1;
    }
}

int cm_uart_send_until(int fd, unsigned char *send_buf, int data_len)
{
    int len = 0;

    int remain = data_len;
    int send_block = 2000;
    int sended = 0;
    while (remain > 0)
    {
        if (remain < send_block)
        {
            send_block = remain;
        }
        len = write(fd, send_buf + sended, send_block);
        if (len <= 0)
        {
            break;
        }
        remain -= len;
        sended += len;
    }
    tcdrain(fd);
    return data_len - remain;
}