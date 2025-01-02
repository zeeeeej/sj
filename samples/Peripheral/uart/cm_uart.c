//
// Created by v on 18-6-21.
//

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#include "cm_uart.h"

int try_set_baudrate(int fd, speed_t baudrate) {
    struct termios options;
    if (tcgetattr(fd, &options) != 0) {
        perror("tcgetattr");
        return -1;
    }

    if (cfsetispeed(&options, baudrate) != 0 || cfsetospeed(&options, baudrate) != 0) {
        return -1;
    }

    if (tcsetattr(fd, TCSANOW, &options) != 0) {
        return -1;
    }

    return 0;
}

    speed_t baudrates[] = {
        B0, B50, B75, B110, B134, B150, B200, B300, B600, B1200,
        B1800, B2400, B4800, B9600, B19200, B38400, B57600, B115200,
        B230400, B460800, B500000, B576000, B921600, B1000000, B1152000,
        B1500000, B2000000, B2500000, B3000000, B3500000, B4000000
    };
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