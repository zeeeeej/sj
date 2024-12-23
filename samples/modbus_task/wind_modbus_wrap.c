
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "cm_common.h"
#include "wind_modbus_wrap.h"

typedef struct
{
    modbus_t *ctx;
} WindModbusHandle;

static WindModbusHandle s_wind_modbus_handle;

void modbus_rtu_custom_rts_control(modbus_t *ctx, int on)
{
    if (on)
    {
        char buf[1024] = {0};
        snprintf(buf, sizeof(buf), "echo 1 > /sys/class/gpio/gpio53/value");
        system(buf);
    }
    else
    {
        char buf[1024] = {0};
        snprintf(buf, sizeof(buf), "echo 0 > /sys/class/gpio/gpio53/value");
        system(buf);
    }
}

static int wind_connect_uart_ready()
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

int wind_modbus_wrap_init()
{
    wind_connect_uart_ready();
    s_wind_modbus_handle.ctx = modbus_new_rtu("/dev/ttyS0", 230400, 'N', 8, 1);
    if (!s_wind_modbus_handle.ctx)
    {
        LOGD("modbus rtc init failed");
        return -1;
    }

    modbus_set_slave(s_wind_modbus_handle.ctx, 1);
    modbus_rtu_set_custom_rts(s_wind_modbus_handle.ctx, modbus_rtu_custom_rts_control);
    modbus_rtu_set_rts(s_wind_modbus_handle.ctx, MODBUS_RTU_RTS_UP);
    modbus_rtu_set_serial_mode(s_wind_modbus_handle.ctx, MODBUS_RTU_RS485);

    modbus_set_indication_timeout(s_wind_modbus_handle.ctx, 0, 100000);
    int time_us = modbus_rtu_get_rts_delay(s_wind_modbus_handle.ctx);
    modbus_rtu_set_rts_delay(s_wind_modbus_handle.ctx, time_us * 200);
    LOGD("delay time us %d", time_us);

    modbus_set_debug(s_wind_modbus_handle.ctx, 1);
    modbus_set_error_recovery(
        s_wind_modbus_handle.ctx, MODBUS_ERROR_RECOVERY_LINK | MODBUS_ERROR_RECOVERY_PROTOCOL);

    int rc = modbus_connect(s_wind_modbus_handle.ctx);
    if (rc < 0)
    {
        LOGD("modbus connect failed [%s]", modbus_strerror(errno));
        modbus_free(s_wind_modbus_handle.ctx);
        s_wind_modbus_handle.ctx = NULL;
        return -2;
    }

    int fd = modbus_get_socket(s_wind_modbus_handle.ctx);
    int flags = fcntl(fd, F_GETFL, 0);
    flags &= ~O_NONBLOCK;
    fcntl(fd, F_SETFL, flags);

    modbus_flush(s_wind_modbus_handle.ctx);
    LOGD("modbus init success");
    return 0;
}

int wind_modbus_wrap_deinit()
{
    modbus_flush(s_wind_modbus_handle.ctx);
    modbus_free(s_wind_modbus_handle.ctx);
    s_wind_modbus_handle.ctx = NULL;
    return 0;
}

int wind_modbus_wrap_recv(unsigned char *buf, int len)
{
    int recv_size = 0;
    int offest = 0;
    do
    {
        recv_size = modbus_receive(s_wind_modbus_handle.ctx, &buf[offest]);
        if (recv_size < 0)
        {
            // LOGD("modbus recv failed [%s] or timeout", modbus_strerror(errno));
            // return -1;
        }
    } while (recv_size <= 0);
    return recv_size;
}

int wind_modbus_wrap_send(unsigned char *buf, int len)
{
    int send_size = send_msg(s_wind_modbus_handle.ctx, buf, len);
    return send_size;
}