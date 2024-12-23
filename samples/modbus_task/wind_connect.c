//
// Created by DELL on 24-9-20.
//

#include <stdio.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <string.h>

#include "wind_connect.h"
#include "cm_config.h"
#include "cm_common.h"
#include "cm_list.h"
#include "cm_utils.h"
#include "cm_uart.h"
#include "wind_protocol_wrap.h"
#include "wind_process_manager.h"
#include "door_detect.h"


enum {
    CONNECT_READ_NONE = 0,
    CONNECT_READ_START = 1,
    CONNECT_READ_CMD = 2,
    CONNECT_READ_SUBCMD = 3,
    CONNECT_READ_ARGS = 4,
};

typedef struct {
    int cmd;
    int sub_cmd;

    int angle_door;
    int angle_image;
    int door_timeout;
} ConnectCmd;

typedef struct {
    struct list_head test_list_item;
    char data[128];
    size_t  len;
} RsData;

typedef struct {
    pthread_t thread;
    int       exited;

    char dev[32];
    int bs;
    int uart_fd;
    RsData   rs_data;

    int read_status;
    ConnectCmd cur_cmd;
} WindConnectHandler;

static WindConnectHandler s_wind_handler;

static void wind_connect_status_changed(int st)
{
    LOGD("wind_connect_status_changed [%d] => [%d]", s_wind_handler.read_status, st);
    s_wind_handler.read_status = st;
}

void enable_uart_send()
{
    char buf[1024] = { 0 };
    snprintf(buf, sizeof(buf), "echo 1 > /sys/class/gpio/gpio53/value");
    system(buf);
}

void enable_uart_recv()
{
    char buf[1024] = { 0 };
    snprintf(buf, sizeof(buf), "echo 0 > /sys/class/gpio/gpio53/value");
    system(buf);
}

static int wind_connect_send_data_now(unsigned char *data, int len)
{
    enable_uart_send();
    int ret = cm_uart_send(s_wind_handler.uart_fd, (unsigned char *) data, len);
    if (ret <= 0) {
        LOGD("send data failed len [%d]", len);
    }
    return 0;
}

static int wind_rs_data_push(unsigned char *data, size_t len)
{
    if (len > 128)
    {
        LOGD("data len is too long max 128");
        return -1;
    }
    RsData *rs_data = (RsData *)cm_mem_malloc(sizeof(RsData));
    memcpy(rs_data->data, data, len);
    rs_data->len = len;
    LOGD("push data: %d", rs_data->len);
    cm_list_add_tail(&rs_data->test_list_item, &s_wind_handler.rs_data.test_list_item);
    return 0;
}

//static int wind_rs_data_push_rsdata(RsData *data)
//{
//    cm_list_add_tail(&data->test_list_item, &s_wind_handler.rs_data.test_list_item);
//    return 0;
//}

static RsData *wind_rs_data_pop() {
    struct list_head *pos = NULL;
    struct list_head *n = NULL;
    RsData *rs_data = NULL;
    cm_list_each_safe(pos, n, &s_wind_handler.rs_data.test_list_item)
    {
        rs_data = cm_list_item(pos, RsData, test_list_item);
        if (rs_data) {
            LOGD("pop data: %d", rs_data->len);
            cm_list_del_item(&rs_data->test_list_item);
            return rs_data;
        }
    }
    return NULL;
}

static int wind_connect_transfer_image_file(const char *filename, size_t file_size)
{
    FILE *f = fopen(filename, "r");
    if (!f)
    {
        return -1;
    }
    size_t transfered = 0;
    size_t block_size = 1024;
    unsigned char send_data[1024] = {0};
    while (transfered < file_size) {
        size_t remain = file_size - transfered;
        if (remain > block_size-1) {
            remain = block_size-1;
        }
        size_t len = fread(send_data, 1, remain, f);
        if (len > 0) {
            int ret = cm_uart_send(s_wind_handler.uart_fd, send_data, (int) len);
            if (ret <= 0) {
                LOGD("send data failed len [%d]", remain);
                fclose(f);
                return -1;
            }
            transfered += ret;
        }
    }
    fclose(f);
    LOGD("transfer image file [%s] success of len [%zu]", filename, transfered);

    return 0;
}

static int wind_connect_find_transfer_image()
{
    unsigned char data[8] = {0};
    int c = 0;
    int count = 0, angle = 0;
    while (c++ < 3) {
        int ret = cm_uart_recv(s_wind_handler.uart_fd, data, 4, 1, 0);
        if (ret > 0) {
            count = wind_protocol_get_open_count(data, 4);
            LOGD("get count: %d\n", count);
            if (count > 0)
            {
                int j = 0;
                while (j++ < 3) {
                    unsigned char rdata[4] = {0};
                    ret = cm_uart_recv(s_wind_handler.uart_fd, rdata, 1, 1, 0);
                    if (ret > 0) {
                        angle = wind_protocol_get_angle(rdata, 1);
                        break;
                    }
                }
                break;
            }
        }
    }
    // goto find if file exist
    char image_path[128] = {0};
    snprintf(image_path, sizeof(image_path), "/tmp/data/%d/img-%d-%d.jpg", count, count, angle);
    if (access(image_path, F_OK) == 0)
    {
        unsigned char resp[16] = {0};
        size_t image_size = cm_file_size(image_path);
        int len = wind_protocol_transfer_image_start_resp(resp, 16, image_size);
        wind_connect_send_data_now(resp, len);
        usleep(50000);
        wind_connect_transfer_image_file(image_path, image_size);
        usleep(50000);
        len = wind_protocol_transfer_image_end_resp(resp, 16);
        wind_connect_send_data_now(resp, len);
        sleep(1);
    }
    else
    {
        LOGD("image [%s] not exist", image_path);
        unsigned char resp[8] = {0};
        int len = wind_protocol_transfer_image_resp_fail(resp, 8);
        wind_rs_data_push(resp, len);
    }
    return 0;
}

static int wind_connect_set_angle_image()
{
    if (s_wind_handler.cur_cmd.sub_cmd == WIND_PROTOCOL_CMD_SUB_SET) {
        LOGD("response set angle image\n");
        unsigned char buf3[8] = {0};
        int c = 0;
        while (c++ < 3) {
            int ret = cm_uart_recv(s_wind_handler.uart_fd, buf3, 1, 1, 0);
            if (ret > 0) {
                int angle = wind_protocol_get_angle(buf3, 1);
                LOGD("get angle: %d\n", angle);
                unsigned char resp[10] = {0};
                int len = 0;
                if (angle > 0 && angle < 180) {
                    cm_config_set_angle_max((float) angle);
                    len = wind_protocol_set_resp(resp, 10, WIND_PROTOCOL_CMD_SET_ANGLE_IMAGE,
                                                 WIND_PROTOCOL_CMD_SUB_SET, 1);
                } else {
                    len = wind_protocol_set_resp(resp, 10, WIND_PROTOCOL_CMD_SET_ANGLE_IMAGE,
                                                 WIND_PROTOCOL_CMD_SUB_SET, 0);
                }
                wind_rs_data_push(resp, len);
                break;
            }
        }
    } else {
        float angle = 0;
        cm_config_get_angle_max(&angle);
        LOGD("response get angle image [%f]\n", angle);
        unsigned char resp[10] = {0};
        int len = wind_protocol_get_resp_angle_image(resp, 10, (int)angle);
        wind_rs_data_push(resp, len);
    }
    return 0;
}

static int wind_connect_set_angle_open()
{
    if (s_wind_handler.cur_cmd.sub_cmd == WIND_PROTOCOL_CMD_SUB_SET) {
        LOGD("response set angle open\n");
        unsigned char buf3[8] = {0};
        int c = 0;
        while (c++ < 3) {
            int ret = cm_uart_recv(s_wind_handler.uart_fd, buf3, 1, 1, 0);
            if (ret > 0) {
                int angle = wind_protocol_get_angle(buf3, 1);
                LOGD("get angle: %d\n", angle);
                unsigned char resp[10] = {0};
                int len = 0;
                if (angle > 0 && angle < 180) {
                    cm_config_set_angle_open((float) angle);
                    cm_config_set_angle_close((float) angle);
                    len = wind_protocol_set_resp(resp, 10, WIND_PROTOCOL_CMD_SET_ANGLE_OPEN,
                                                 WIND_PROTOCOL_CMD_SUB_SET, 1);
                } else {
                    len = wind_protocol_set_resp(resp, 10, WIND_PROTOCOL_CMD_SET_ANGLE_OPEN,
                                                 WIND_PROTOCOL_CMD_SUB_SET, 0);
                }
                wind_rs_data_push(resp, len);
                break;
            }
        }
    } else {
        LOGD("response get angle open\n");
        float angle = 0;
        cm_config_get_angle_open(&angle);
        unsigned char resp[10] = {0};
        int len = wind_protocol_get_resp_angle_open(resp, 10, (int)angle);
        wind_rs_data_push(resp, len);
    }
    return 0;
}

static int wind_connect_set_open_timeout()
{
    if (s_wind_handler.cur_cmd.sub_cmd == WIND_PROTOCOL_CMD_SUB_SET) {
        LOGD("response set timeout\n");
        unsigned char buf3[8] = {0};
        int c = 0;
        while (c++ < 3) {
            int ret = cm_uart_recv(s_wind_handler.uart_fd, buf3, 1, 2, 0);
            if (ret > 0) {
                int timeout = wind_protocol_get_open_timeout(buf3, 2);
                LOGD("get timeout: %d\n", timeout);
                unsigned char resp[10] = {0};
                int len = 0;
                ret = cm_config_set_door_open_timeout(timeout);
                if (ret >= 0) {
                    len = wind_protocol_set_resp(resp, 10, WIND_PROTOCOL_CMD_SET_OPEN_TIMEOUT,
                                                 WIND_PROTOCOL_CMD_SUB_SET, 1);
                } else {
                    len = wind_protocol_set_resp(resp, 10, WIND_PROTOCOL_CMD_SET_OPEN_TIMEOUT,
                                                 WIND_PROTOCOL_CMD_SUB_SET, 0);
                }
                wind_rs_data_push(resp, len);
                break;
            }
        }
    } else {
        int timeout = cm_config_get_door_open_timeout();
        LOGD("response get door timeout [%d]\n", timeout);
        unsigned char resp[10] = {0};
        int len = wind_protocol_get_resp_door_timeout(resp, 10, timeout);
        wind_rs_data_push(resp, len);
    }
    return 0;
}

// static int wind_connect_usb_connect(int subcmd)
// {
//     if (subcmd == WIND_PROTOCOL_CMD_SUB_SET)
//     {
//         LOGD("close media");
//         door_deinit();
//         LOGD("open media");
//         start_usb_connect();
//     }
//     else
//     {
//         LOGD("stop media and start normal media");
//         stop_usb_connect();
//         door_init();
//     }

//     return 0;
// }

static int wind_connect_recv_data_parse_and_do()
{
    if (s_wind_handler.cur_cmd.cmd == WIND_PROTOCOL_CMD_GET_VERSION)
    {
        unsigned char resp[10] = {0};
        int len = wind_protocol_resp_version(resp, 10);
        wind_rs_data_push(resp, len);
        LOGD("send version and read again len [%d]\n", len);
        wind_connect_status_changed(CONNECT_READ_NONE);
    }
    else if (s_wind_handler.cur_cmd.cmd == WIND_PROTOCOL_CMD_SET_ANGLE_IMAGE)
    {
        wind_connect_set_angle_image();
        wind_connect_status_changed(CONNECT_READ_NONE);
    } else if (s_wind_handler.cur_cmd.cmd == WIND_PROTOCOL_CMD_GET_IMAGE)
    {
        LOGD("response get image\n");
        wind_connect_find_transfer_image();
        wind_connect_status_changed(CONNECT_READ_NONE);
    } else if (s_wind_handler.cur_cmd.cmd == WIND_PROTOCOL_CMD_SET_ANGLE_OPEN)
    {
        wind_connect_set_angle_open();
        wind_connect_status_changed(CONNECT_READ_NONE);
    } else if (s_wind_handler.cur_cmd.cmd == WIND_PROTOCOL_CMD_SET_OPEN_TIMEOUT)
    {
        wind_connect_set_open_timeout();
        wind_connect_status_changed(CONNECT_READ_NONE);
    } else if (s_wind_handler.cur_cmd.cmd == WIND_PROTOCOL_CMD_SET_USB_CONNECT)
    {
        // wind_connect_usb_connect(s_wind_handler.cur_cmd.sub_cmd);
        wind_connect_status_changed(CONNECT_READ_NONE);
    }
    return 0;
}

static int wind_connect_recv_data_continue()
{
    switch (s_wind_handler.read_status) {
        case CONNECT_READ_NONE:
        {
            int ret = 0;
            unsigned char buf[2] = {0};
            // do {
            ret = cm_uart_recv(s_wind_handler.uart_fd, buf, 1, 0, 50);
            if (ret > 0)
            {
                    LOGD("Recv data from uart: %02x", buf[0]);
                if (cm_is_debug())
                {
                    LOGD("Recv data from uart: %02x", buf[0]);
                }
                if (wind_protocol_is_head(buf, 2))
                {
                    // wind_connect_status_changed(CONNECT_READ_CMD);
                }
            }
            // } while (ret > 0);
        }
            break;
        case CONNECT_READ_START:
            s_wind_handler.read_status = CONNECT_READ_NONE;
            break;
        case CONNECT_READ_CMD: {
            unsigned char buf[2] = {0};
            int ret = cm_uart_recv(s_wind_handler.uart_fd, buf, 1, 0, 5000);
            if (ret > 0) {
                if (wind_protocol_is_cmd_get_image(buf, 2)) {
                    wind_connect_status_changed(CONNECT_READ_SUBCMD);
                    s_wind_handler.cur_cmd.cmd = WIND_PROTOCOL_CMD_GET_IMAGE;
                } else if (wind_protocol_is_cmd_get_version(buf, 2)) {
                    wind_connect_status_changed(CONNECT_READ_SUBCMD);
                    s_wind_handler.cur_cmd.cmd = WIND_PROTOCOL_CMD_GET_VERSION;
                } else if (wind_protocol_is_cmd_set_angle_image(buf, 2)){
                    wind_connect_status_changed(CONNECT_READ_SUBCMD);
                    s_wind_handler.cur_cmd.cmd = WIND_PROTOCOL_CMD_SET_ANGLE_IMAGE;
                } else if (wind_protocol_is_cmd_set_angle_open(buf, 2)){
                    wind_connect_status_changed(CONNECT_READ_SUBCMD);
                    s_wind_handler.cur_cmd.cmd = WIND_PROTOCOL_CMD_SET_ANGLE_OPEN;
                } else if (wind_protocol_is_cmd_set_open_timeout(buf, 2)){
                    wind_connect_status_changed(CONNECT_READ_SUBCMD);
                    s_wind_handler.cur_cmd.cmd = WIND_PROTOCOL_CMD_SET_OPEN_TIMEOUT;
                } else if (wind_protocol_is_cmd_set_usb_connect(buf, 2)){
                    wind_connect_status_changed(CONNECT_READ_SUBCMD);
                    s_wind_handler.cur_cmd.cmd = WIND_PROTOCOL_CMD_SET_USB_CONNECT;
                }
            }
        }
            break;
        case CONNECT_READ_SUBCMD:{
            unsigned char buf[2] = {0};
            int ret = cm_uart_recv(s_wind_handler.uart_fd, buf, 1, 0, 5000);
            if (ret > 0) {
                s_wind_handler.cur_cmd.sub_cmd = wind_protocol_is_sub_cmd_set(buf, 2);

                wind_connect_status_changed(CONNECT_READ_ARGS);
            }
        }
            break;
        case CONNECT_READ_ARGS:
            wind_connect_recv_data_parse_and_do();
            break;
    }
    return 0;
}

static int wind_connect_transfer()
{
    // 已经在读取了，就不要暂停去发送，直到接收完成
    if (s_wind_handler.read_status == CONNECT_READ_NONE) {
        int c = 0;
        if (!cm_list_empty(&s_wind_handler.rs_data.test_list_item)) {
            enable_uart_send();
        }
        while (c++ < 10) {
            RsData *rs_data = wind_rs_data_pop();
            if (rs_data) {
                // send data out
                int ret = cm_uart_send(s_wind_handler.uart_fd, (unsigned char *) rs_data->data, rs_data->len);
                if (ret < 0) {
                    LOGD("send data failed\n");
                }
                else
                {
                    LOGD("send data success len [%d]", rs_data->len);
                }
                cm_mem_free(rs_data);
                // 加延迟是为了让两条消息分开一些，但实际上可以不用这样，接收端要按序解析就可以连续发
                usleep(50000);
            } else {
                break;
            }
        }
    }

    enable_uart_recv();
    wind_connect_recv_data_continue();
    return 0;
}

static int wind_connect_uart_open()
{
    int ret = cm_uart_open(s_wind_handler.dev);
    if (ret > 0) {
        s_wind_handler.uart_fd = ret;
        ret = cm_uart_init(ret, s_wind_handler.bs, 0, 8, 1, 'n');
        if (ret < 0)
        {
            LOGD("uart init failed\n");
            cm_uart_close(s_wind_handler.uart_fd);
        }
        else
        {
            LOGD("uart init success dev [%s]", s_wind_handler.dev);
            return 0;
        }
    }
    else
    {
        LOGD("ERROR: open uart [%s] failed ret [%d]\n", s_wind_handler.dev, ret);
    }
    return -1;
}

void *wind_connect_thread(void *arg)
{
    prctl(PR_SET_NAME, "wind_connect", 0, 0, 0);
    WindConnectHandler *handler = (WindConnectHandler *)arg;
    LOGD("wind_connect thread start");

    while (1)
    {
        int ret = wind_connect_uart_open();
        if (ret < 0)
        {
            LOGD("open uart failed, retry after 500ms");
            usleep(1000 * 500);
        }
        else
        {
            break;
        }
    }

    while (!handler->exited)
    {
        if (wind_process_manager_get_status() == PROCESS_STATUS_IDLE ||
                wind_process_manager_get_status() == PROCESS_STATUS_DOOR_OPEN) {
            wind_connect_transfer();
        }

        usleep(10000);
    }
    LOGD("wind_connect thread exit\n");
    return NULL;
}

int wind_connect_uart_ready()
{
    if (access("/sys/class/gpio/gpio53", F_OK) != 0)
    {
        LOGD("init serial port gpio53\n");
        char buf[1024] = { 0 };
        snprintf(buf, sizeof(buf), "echo 53 > /sys/class/gpio/export");
        system(buf);
        snprintf(buf, sizeof(buf), "echo out > /sys/class/gpio/gpio53/direction");
        system(buf);
    }
    return 0;
}

static int wind_connect_init_with_thread()
{
    s_wind_handler.exited = 0;
    wind_connect_status_changed(CONNECT_READ_NONE);
    int ret = pthread_create(&s_wind_handler.thread, NULL, wind_connect_thread, &s_wind_handler);
    if (ret < 0)
    {
        LOGD("Error: thread create failed\n");
        return -1;
    }
    cm_init_list_head(&s_wind_handler.rs_data.test_list_item);
    LOGD("wind_connect thread init");
    return 0;
}

int wind_connect_init_in_thread()
{
    cm_strncpy(s_wind_handler.dev, "/dev/ttyS0", sizeof(s_wind_handler.dev));
    s_wind_handler.bs = 1200;
    wind_connect_uart_ready();

    return wind_connect_init_with_thread();
}

int wind_connect_deinit()
{
    s_wind_handler.exited = 1;
    pthread_join(s_wind_handler.thread, NULL);
    LOGD("wind_connect thread deinit");
    return 0;
}

int wind_connect_send_data(const char *data, int len)
{
    return 0;
}

int wind_connect_door_open(unsigned long long count)
{
    unsigned char data[10] = {0};
    int len = wind_protocol_resp_report_door_open(data, 10, count);
    wind_rs_data_push(data, len);
    return 0;
}

int wind_connect_door_close(DoorInfoItem *item)
{
    unsigned char data[10] = {0};
    int len = wind_protocol_resp_report_door_close(data, 10, (int)item->image_angle, item->open_count);
    wind_rs_data_push(data, len);
    return 0;
}
