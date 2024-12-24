
#include "wind_connect_up.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>  
#include <sys/reboot.h>
#include <linux/reboot.h>
#include <sys/syscall.h>
#include "wind_modbus_wrap.h"
#include "cm_common.h"
#include "wind_global.h"
#include "wind_protocol_wrap_up.h"
#include "wind_usb_connect.h"
#include "cm_list.h"
#include "cm_config.h"
#include "cm_utils.h"
#include "door_detect.h"
#include "cm_uart.h"
#include "cJSON.h"
#include "cm_video_ctrl.h"
#include <sys/wait.h>
#include <termios.h>
#include <sys/time.h>
#include <time.h>
#include "ota_process.h"     
#include "take_photo.h"
#include "trans_door_image.h"
#include "circular_log.h"
static char* TAG_NAME = "modbus_msg";

// 添加日志标签定义
#define MODBUS_TAG "MODBUS"

// 添加日志宏定义，只保留关键日志级别
#define MODBUS_LOG_ERROR(fmt, ...) log_write(LOG_ERROR, MODBUS_TAG, fmt, ##__VA_ARGS__)
#define MODBUS_LOG_INFO(fmt, ...)  log_write(LOG_INFO, MODBUS_TAG, fmt, ##__VA_ARGS__)

/**/
static int reboot_flag = 0;
#define REBOOT_COUNTDOWN_SECONDS 3
WindProOtaRequest ota_req;
#define DEST_PATH "/system/bin/sample_camera_rst"
#define MD5_BUF_SIZE 4096
#define OTA_FILE_PATH "tmp/ota_app/sample_camera_rst.xz" // 文件保存路径
#define OTA_FILE_XZ_AF_PATH "tmp/ota_app/sample_camera_rst"
static FILE *fp = NULL;
static uint32_t received_size = 0;
/* Table of CRC values for high-order byte */
static const uint8_t table_crc_hi[] = {
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1,
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1,
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40};

/* Table of CRC values for low-order byte */
static const uint8_t table_crc_lo[] = {
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5,
    0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B,
    0xC9, 0x09, 0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE,
    0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6,
    0xD2, 0x12, 0x13, 0xD3, 0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,
    0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D,
    0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8,
    0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C,
    0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26, 0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21,
    0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,
    0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE, 0xAA, 0x6A,
    0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA,
    0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5, 0x77, 0xB7,
    0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0, 0x50, 0x90, 0x91, 0x51,
    0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
    0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98,
    0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D,
    0x4C, 0x8C, 0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83,
    0x41, 0x81, 0x80, 0x40};

typedef struct
{
    int bexit;
    pthread_t pid;
    WindProUpdateHeader cur_header;
    int uart_fd;

    WindSystemEvent event_list;
} WindConnectUpHandler;

static WindConnectUpHandler s_wind_connect_up;

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

static int wind_connect_uart_open()
{
    char *dev = "/dev/ttyS0";
    int ret = cm_uart_open(dev);
    if (ret > 0)
    {
        s_wind_connect_up.uart_fd = ret;
        ret = cm_uart_init(ret, 230400, 0, 8, 1, 'n');
        if (ret < 0)
        {
            LOGD("uart init failed\n");
            cm_uart_close(s_wind_connect_up.uart_fd);
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

static int wind_connect_uart_close()
{
    if (s_wind_connect_up.uart_fd > 0)
    {
        cm_uart_close(s_wind_connect_up.uart_fd);
        s_wind_connect_up.uart_fd = -1;
    }
    return 0;
}
static void print_timestamp(const char *prefix)
{
    struct timeval tv;
    struct tm timeinfo;
    char strftime_buf[64];
    char complete_buf[128];

    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
    snprintf(complete_buf, sizeof(complete_buf), "%s.%06ld", strftime_buf, tv.tv_usec);

    LOGD("%s: %s", prefix, complete_buf);
}
static int wind_connect_uart_read(unsigned char *data, int len)
{

    /*test*/
    // int ret = cm_uart_recv_simple(s_wind_connect_up.uart_fd, data, 1);
    // if (ret > 0)
    // {
    //     LOGD("read data: %d,ret : %d", data[0], ret);
    //     usleep(10000);
    //     // enable_uart_send();
    //     // int ret = cm_uart_send_until(s_wind_connect_up.uart_fd, data, 1);
    //     //     LOGD("send data: %d", ret);
    //     return 1;
    // }
    // return 0;

    if (s_wind_connect_up.uart_fd <= 0)
    {
        return -1;
    }
    int step = 1;
    int c = 0;
    int pos = 0;
    int data_len = 0;

    while (1)
    {
        switch (step)
        {
        case 1:
        {
            cm_uart_recv_simple(s_wind_connect_up.uart_fd, data, 1);
            c++;
            if (data[0] == 0x01)
            {   
                // printf("data[0] == 0x01\n");
                c = 0;
                step = 2;
            }
        }
        break;
        case 2:
            cm_uart_recv_simple(s_wind_connect_up.uart_fd, data + 1, 1);
            c++;
            if (data[1] == 0x17)
            {
                // printf("data[1] == 0x17\n");
                c = 0;
                step = 3;
                pos += 2;
            }
            else if (data[1] == 0x01)
            {
                step = 2;
            }
            else
            {
                c = 0;
                step = 1;
            }
            break;
        case 3:
        {
            int r = cm_uart_recv_simple(s_wind_connect_up.uart_fd, data + pos, 12 - pos);
            if (r < 0)
            {
                break;
            }
            pos += r;
            if (pos >= 12)
            {
                data_len = (data[10] << 8) | (data[11]);
                data_len += 2; // crc
                // LOGD("data len [%d]", data_len);
                if (data_len > 8000)
                {
                    LOGD("invalid data len [%d]", data_len);
                    pos = 0;
                    step = 1;
                }
                else
                {
                    step = 4;
                }
                break;
            }
        }
        break;
        case 4:
        {
            int r = cm_uart_recv_simple(s_wind_connect_up.uart_fd, data + pos, data_len - pos + 12);
            if (r < 0)
            {
                break;
            }
            pos += r;
            if (pos >= data_len + 12)
            {
                return pos;
            }
        }
        break;
        default:
            step = 1;
            break;
        }
    }
    return 0;
}

static uint16_t crc16(uint8_t *buffer, uint16_t buffer_length)
{
    uint8_t crc_hi = 0xFF; /* high CRC byte initialized */
    uint8_t crc_lo = 0xFF; /* low CRC byte initialized */
    unsigned int i;        /* will index into CRC lookup */

    /* pass through message buffer */
    while (buffer_length--)
    {
        i = crc_lo ^ *buffer++; /* calculate the CRC  */
        crc_lo = crc_hi ^ table_crc_hi[i];
        crc_hi = table_crc_lo[i];
    }

    return (crc_hi << 8 | crc_lo);
}

static int _modbus_rtu_send_msg_pre(uint8_t *req, int req_length)
{
    uint16_t crc = crc16(req, req_length);

    /* According to the MODBUS specs (p. 14), the low order byte of the CRC comes
     * first in the RTU message */
    req[req_length++] = crc & 0x00FF;
    req[req_length++] = crc >> 8;

    return req_length;
}

#include <time.h>
#define ENABLE_DECODE_TIMING 0
static int wind_connect_uart_write(unsigned char *data, int len)
{
    if (s_wind_connect_up.uart_fd <= 0)
    {
        return -1;
    }
    enable_uart_send();

#if ENABLE_DECODE_TIMING
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
#endif

    len = _modbus_rtu_send_msg_pre((uint8_t *)data, len); // CRC校验在这个函数中完成

#if ENABLE_DECODE_TIMING
    clock_gettime(CLOCK_MONOTONIC, &end);
    double crc_time = (end.tv_sec - start.tv_sec) * 1000.0 +
                      (end.tv_nsec - start.tv_nsec) / 1000000.0; // 转换为毫秒
    printf("CRC calculation time: %.3f ms\n", crc_time);

    // 开始发送计时
    clock_gettime(CLOCK_MONOTONIC, &start);
#endif



    int ret = cm_uart_send_until(s_wind_connect_up.uart_fd, data, len);
// #if WIND_CONNECT_UP_DEBUG
    // for (int i = 0; i < len; i++)
    // {
    //     printf("%02x ", data[i]);
    // }
    // LOGD("send data length : %d\n",ret);
// #endif
#if ENABLE_DECODE_TIMING
    clock_gettime(CLOCK_MONOTONIC, &end);
    double send_time = (end.tv_sec - start.tv_sec) * 1000.0 +
                       (end.tv_nsec - start.tv_nsec) / 1000000.0;
    printf("=== UART Write Timing ===\n");
    printf("Data length: %d bytes\n", len);
    printf("CRC time: %.3f ms\n", crc_time);
    printf("Send time: %.3f ms\n", send_time);
    printf("Total time: %.3f ms\n", crc_time + send_time);
    printf("=====================\n");
#endif
    if (tcflush(s_wind_connect_up.uart_fd, TCIFLUSH) == -1)
    {
        perror("tcflush failed");
        // 进行错误处理
    }
    return ret;
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

static int wind_serial_open()
{
    wind_connect_uart_ready();
    enable_uart_recv();
    // wind_modbus_wrap_init();
    wind_connect_uart_open();
    return 0;
}

static int wind_serial_close()
{
    // wind_modbus_wrap_deinit();
    wind_connect_uart_close();
    return 0;
}

WindSystemEvent *wind_system_event_pop()
{
    struct list_head *pos = NULL;
    struct list_head *n = NULL;
    WindSystemEvent *se = NULL;
    cm_list_each_safe(pos, n, &s_wind_connect_up.event_list.test_list_item)
    {
        se = cm_list_item(pos, WindSystemEvent, test_list_item);
        if (se)
        {
            LOGD("pop data: %d, cmd: %d", se->info.open_count, se->info.cmd);
            cm_list_del_item(&se->test_list_item);
            return se;
        }
    }
    return NULL;
}

static int wind_serial_operate_version(unsigned char *res, int len, WindProUpdateHeader *header)
{
    return wind_protocol_wrap_up_gen_response_version((char *)res, len, header, "signel_disp_v1.0.1");
}

static int wind_serial_system_event(unsigned char *res, int len, WindProUpdateHeader *header)
{
    // 队列里面 pop 出一个数据
    WindSystemEvent *event = wind_system_event_pop();
    if (event)
    {
        WindProUpdateSystemInfo info;
        info.header = *header;
        info.header.result = 0;
        info.cmd = event->info.cmd;
        info.angle = event->info.angle;
        info.open_count = event->info.open_count;
        free(event);
        return wind_protocol_wrap_up_gen_response_query_system((char *)res, len, &info);
    }
    else
    {
        WindProUpdateSystemInfo info;
        info.header = *header;
        info.header.result = 99;
        return wind_protocol_wrap_up_gen_response_query_system((char *)res, len, &info);
    }
}

static int wind_serial_query_config(unsigned char *res, int len, WindProUpdateHeader *header)
{
    WindProUpdateConfig config;
    config.header = *header;
    float angle_max = 36.0;
    cm_config_get_angle_max(&angle_max);
    config.capture_angle = (int)angle_max;
    float angle_open = 4.0;
    cm_config_get_angle_open(&angle_open);
    config.door_open_angle = angle_open;
    config.door_open_timeout = cm_config_get_door_open_timeout();
    config.header.result = 0;
    return wind_protocol_wrap_up_gen_response_query_config((char *)res, len, &config);
}

static int wind_serial_set_config(unsigned char *res, int len, char *data, int data_len, WindProUpdateHeader *header)
{
    WindProUpdateConfig config;
    memset(&config, 0, sizeof(WindProUpdateConfig));
    config.header = *header;
    wind_protocol_wrap_up_parse_request_set_config(data, data_len, &config);
    if ((config.capture_angle <= 0 || config.capture_angle > 180) ||
        (config.door_open_angle <= 0 || config.door_open_angle > 180) ||
        (config.door_open_timeout <= 0 || config.door_open_timeout > 65535) ||
        (config.capture_angle < config.door_open_angle))
    {
        config.header.result = 1;
        return wind_protocol_wrap_up_gen_response_set_config((char *)res, len, &config, config.header.result);
    }
    else
    {
        cm_config_set_angle_max(config.capture_angle);
        cm_config_set_angle_open(config.door_open_angle);
        cm_config_set_angle_close(config.door_open_angle);
        cm_config_set_door_open_timeout(config.door_open_timeout);
        config.header.result = 0;
        return wind_protocol_wrap_up_gen_response_set_config((char *)res, len, &config, config.header.result);
    }
}

void delete_img(char *img_path)
{
    char cmd_remove_path[256] = {0};
    snprintf(cmd_remove_path, sizeof(cmd_remove_path), "rm -rf %s", img_path);
    LOGD("remove [%s]\n", cmd_remove_path);
    system(cmd_remove_path);
}

int wind_serial_query_self_check_result(unsigned char *res, int len, WindProUpdateHeader *header)
{
    return wind_protocol_wrap_up_gen_response_self_check_result((char *)res, len,header);
}
/**
 * @brief 计算文件的MD5值
 * @param filepath 文件路径
 * @param md5_str 输出的MD5字符串
 * @return 0:成功 -1:失败
 */

// #include <openssl/md5.h>
// static int calculate_file_md5(const char *filepath, char *md5_str)
// {
//     FILE *file = fopen(filepath, "rb");
//     if (!file) {
//         LOGD("Failed to open file for MD5 calculation");
//         return -1;
//     }

//     unsigned char md5_buf[MD5_BUF_SIZE];
//     unsigned char md5_sum[MD5_DIGEST_LENGTH];  // OpenSSL定义
//     size_t bytes_read;
//     MD5_CTX md5_ctx;

//     MD5_Init(&md5_ctx);

//     while ((bytes_read = fread(md5_buf, 1, MD5_BUF_SIZE, file)) > 0) {
//         MD5_Update(&md5_ctx, md5_buf, bytes_read);
//     }

//     MD5_Final(md5_sum, &md5_ctx);
//     fclose(file);

//     // 转换为十六进制字符串
//     for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
//         sprintf(&md5_str[i * 2], "%02x", md5_sum[i]);
//     }
//     md5_str[32] = '\0';

//     return 0;
// }
#include "md5.h"
int wind_serial_request_ota_data_end(char *res, int len, char *data, int data_len, WindProUpdateHeader *header)
{
    LOGD("recv ota end result request");
    char calculated_md5[33] = {0};
    int verify_result = 1; // 默认成功
    if (fp != NULL)
    {
        fclose(fp);
        fp = NULL;
    }
    LOGD("start cal md5");
    // 验证文件大小
    if (received_size != ota_req.file_size)
    {
        LOGD("File size mismatch: received %u, expected %u",
             received_size, ota_req.file_size);
        verify_result = 0;
        goto end;
    }

    // 计算文件MD5
    if (calculate_file_md5(OTA_FILE_PATH, calculated_md5) != 0)
    {
        LOGD("Failed to calculate file MD5");
        verify_result = 0;
        goto end;
    }

    // 比较MD5值
    if (strcmp(calculated_md5, ota_req.file_md5) != 0)
    {
        LOGD("MD5 mismatch:");
        LOGD("Expected: %s", ota_req.file_md5);
        LOGD("Calculated: %s", calculated_md5);
        verify_result = 0;
        goto end;
    }

    LOGD("File MD5 verification passed");
    // 解压缩文件
    // 使用 system 调用解压缩文件

    // // 重新上电
    // LOGD("Rebooting system...");
    // sync(); // 确保所有文件操作完成
    // reboot(RB_AUTOBOOT); // 重新启动系统
    reboot_flag = 1;
end:
    // 根据验证结果生成响应
    return wind_protocol_wrap_up_gen_response_ota_end_result((char *)res, len, header, verify_result);
}
// 添加一个辅助函数来创建目录
static int ensure_directory_exists(const char *path)
{
    char dir_path[256] = {0};
    char *last_slash = strrchr(path, '/');

    if (last_slash)
    {
        strncpy(dir_path, path, last_slash - path);

        // 使用mkdir递归创建目录，权限设置为0777
        char cmd[512] = {0};
        snprintf(cmd, sizeof(cmd), "mkdir -p %s", dir_path);
        return system(cmd);
    }
    return 0;
}
int wind_serial_request_ota_data_trans_packet(char *res, int len, char *data, int data_len, WindProUpdateHeader *header)
{
    LOGD("recv OTA app data...");

    // 解析OTA数据包
    WindProOtaInfo ota_packet_info = {0}; // 使用结构体初始化
    if (wind_protocol_wrap_up_parse_ota_data_trans_packet(data, data_len, &ota_packet_info) != 0)
    {
        LOGD("Failed to parse OTA data packet");
        return -1;
    }

    // 打印数据包信息
    LOGD("OTA packet - offset: %u, length: %u, total_size: %u",
         ota_packet_info.offset, ota_packet_info.length, ota_packet_info.total_size);

    // 验证数据长度
    if (ota_packet_info.length > OTA_BUFFER_SIZE)
    {
        LOGD("Data length too large: %u", ota_packet_info.length);
        return -1;
    }

    // 如果是第一个数据包（offset为0），创建或覆盖文件
    if (ota_packet_info.offset == 0)
    {
        if (fp != NULL)
        {
            fclose(fp);
            fp = NULL;
        }
        // 确保目录存在
        if (ensure_directory_exists(OTA_FILE_PATH) != 0)
        {
            LOGD("Failed to create directory for OTA file");
            return -1;
        }

        fp = fopen(OTA_FILE_PATH, "wb");
        if (fp == NULL)
        {
            LOGD("Failed to create OTA file");
            return -1;
        }
        received_size = 0;
        LOGD("Created new OTA file: %s", OTA_FILE_PATH);
    }

    // 验证文件指针
    if (fp == NULL)
    {
        LOGD("File pointer is NULL");
        return -1;
    }

    // 验证数据完整性
    if (ota_packet_info.offset != received_size)
    {
        LOGD("Data sequence error: expected offset %u, got %u",
             received_size, ota_packet_info.offset);
        fclose(fp);
        fp = NULL;
        return -1;
    }

    // 写入数据
    if (ota_packet_info.length > 0)
    {
        size_t written = fwrite(ota_packet_info.data_buffer, 1, ota_packet_info.length, fp);
        if (written != ota_packet_info.length)
        {
            LOGD("Failed to write data: %u/%u bytes written",
                 written, ota_packet_info.length);
            fclose(fp);
            fp = NULL;
            return -1;
        }
        received_size += written;

        // 打印进度
        int progress = (received_size * 100) / ota_packet_info.total_size;
        LOGD("OTA Progress: %d%% (%u/%u bytes)",
             progress, received_size, ota_packet_info.total_size);

        // 确保数据写入磁盘
        fflush(fp);
    }

    // 如果接收完成，关闭文件
    if (received_size >= ota_packet_info.total_size)
    {
        LOGD("OTA file reception completed: %u bytes", received_size);

        // 确保所有数据写入磁盘
        fflush(fp);
        fclose(fp);
        fp = NULL;

        // 验证文件大小
        long file_size = 0;
        FILE *verify_fp = fopen(OTA_FILE_PATH, "rb");
        if (verify_fp != NULL)
        {
            fseek(verify_fp, 0, SEEK_END);
            file_size = ftell(verify_fp);
            fclose(verify_fp);

            if (file_size != ota_packet_info.total_size)
            {
                LOGD("File size mismatch: %ld != %u",
                     file_size, ota_packet_info.total_size);
                return -1;
            }
            LOGD("File size verified: %ld bytes", file_size);
        }
        else
        {
            LOGD("Failed to verify file size");
            return -1;
        }
    }

    // 生成响应
    return wind_protocol_wrap_up_gen_response_ota_trans_packet(res, len, header);
}

/**
 * @brief 解析OTA升级请求
 * @param data 接收到的JSON数据
 * @param len 数据长度
 * @param header 响应头信息
 * @return 0:成功 -1:失败 -2:部分成功
 */
int wind_serial_request_ota(char *res, int res_len, char *data, int len, WindProUpdateHeader *header)
{
    wind_protocol_wrap_up_parse_request_ota(data, len, &ota_req);
    return wind_protocol_wrap_up_gen_response_ota((char *)res, res_len, header);
}
static void wind_reboot()
{
    int countdown = REBOOT_COUNTDOWN_SECONDS;
    int ret;

    LOGD("========== System Reboot Countdown Started ==========");
    LOGD("System will reboot in %d seconds", countdown);

    // 倒计时
    while (countdown > 0)
    {
        LOGD("Rebooting in %d seconds...", countdown);
        sleep(1);
        countdown--;
    }

    // 执行重启
    LOGD("Executing reboot command NOW!");
    ret = reboot(RB_AUTOBOOT);
    if (ret != 0)
    {
        LOGD("ERROR: Reboot failed: %s (errno: %d)", strerror(errno), errno);
        return;
    }

    // 正常情况下不会执行到这里
    LOGD("WARNING: System still running after reboot command");
}

int wind_serial_request_reboot(unsigned char *res, int len, WindProUpdateHeader *header)
{
    // int ret = -1;

    LOGD("Initiating system reboot sequence...");

    // 同步文件系统
    LOGD("Syncing file systems...");
    sync();
    LOGD("File systems sync completed");

    reboot_flag = 1;
    return wind_serial_reponse_reboot((char *)res, len, header);
}

int wind_serial_query_photo(unsigned char *res, int len, char *data, int data_len, WindProUpdateHeader *header)
{
    WindProUpdateQueryPhoto query_photo;
    memset(&query_photo, 0, sizeof(WindProUpdateQueryPhoto));
    wind_protocol_wrap_up_parse_header(data, data_len, &query_photo.header);
    wind_protocol_wrap_up_parse_request_photo(data, data_len, &query_photo);
    char image_path[128] = {0};
    snprintf(image_path, sizeof(image_path), "/tmp/data/%d/img-%d-%d.jpg",
             query_photo.open_count, query_photo.open_count, query_photo.angle);

    if (access(image_path, F_OK) == 0)
    {
        size_t image_size = cm_file_size(image_path);
        if (query_photo.offset > image_size)
        {
            LOGD("Error: offset [%llu] > image size[%u]", query_photo.offset, image_size);
            query_photo.header.result = 2;
            query_photo.size = image_size;
            query_photo.data_len = 0;
            return wind_protocol_wrap_up_gen_response_photo((char *)res, len, &query_photo, header);
        }
        query_photo.header.result = 0;
        query_photo.size = image_size;
        LOGD("will read file [%s] offset [%llu], length [%llu]", image_path, query_photo.offset, query_photo.length);
        FILE *fp = fopen(image_path, "rb");
        if (fp)
        {
            fseek(fp, query_photo.offset, SEEK_SET);
            query_photo.data_len = fread(query_photo.data, 1, query_photo.length, fp);
            fclose(fp);
            if (query_photo.data_len < 0)
            {
                LOGD("Error: read image fail [%d]", errno);
                query_photo.header.result = 3;
                query_photo.data_len = 0;
                return wind_protocol_wrap_up_gen_response_photo((char *)res, len, &query_photo, header);
            }
            LOGD("read image success len [%d]", query_photo.data_len);

            /* 判断是否读取完，然后删除图片 */
            if (query_photo.offset + query_photo.data_len >= image_size)
            {
                /*读取完毕， 删除图片*/
                LOGD("read all image frame end\n\n");
                delete_img(image_path);
            }
            else
            {
                /*未读取完*/
            }
            /* 判断是否读取完，然后删除图片 */

            if (query_photo.data_len > 8)
            {
                LOGD("read image data [%x %x %x %x] [%x %x %x %x]",
                     query_photo.data[0], query_photo.data[1], query_photo.data[2], query_photo.data[3],
                     query_photo.data[query_photo.data_len - 4], query_photo.data[query_photo.data_len - 3],
                     query_photo.data[query_photo.data_len - 2], query_photo.data[query_photo.data_len - 1]);
            }
            else
            {
                for (int i = 0; i < query_photo.data_len; i++)
                {
                    printf("%02x ", query_photo.data[i]);
                }
                printf("\n");
            }
            LOGD("------ [%d]---------", query_photo.data_len);
            return wind_protocol_wrap_up_gen_response_photo((char *)res, len, &query_photo, header);
        }
        else
        {
            LOGD("Error: open image fail [%d]", errno);
            query_photo.header.result = 4;
            query_photo.data_len = 0;
            return wind_protocol_wrap_up_gen_response_photo((char *)res, len, &query_photo, header);
        }
    }
    else
    {
        LOGD("Error: image [%s] not exist", image_path);
        query_photo.header.result = 1;
        query_photo.size = 0;
        query_photo.data_len = 0;
        return wind_protocol_wrap_up_gen_response_photo((char *)res, len, &query_photo, header);
    }
}
char serial_recv_data[8000] = {0};
void copy_file() {
    char command[256];  // 确保缓冲区足够大以容纳完整命令
    snprintf(command, sizeof(command), "cp %s %s", OTA_FILE_XZ_AF_PATH, DEST_PATH);

    if (system(command) != 0) {
        LOGD("Failed to copy file to destination");
        // verify_result = 0;
        // goto end;
    }
}


static int fill_msg_header(uint8_t* buf,uint16_t cmd_len)
{
    buf[0] = 0xFD;
    buf[1] = 0xFC;
    buf[2] = 0xFB;
    buf[3] = 0xFA;

    // buf[4] = 0x64;  // 101 的低字节 (0x65)
    // buf[5] = 0x00;  // 101 的高字节 (0x00)
    buf[4] = 0x64;  
    buf[5] = 0x00;  
    buf[6] = cmd_len & 0xff;
    buf[7] = (cmd_len >> 8) & 0xff;

    return 8;
}

static int wind_serial_parse_data(unsigned char *buf, int len)
{
    if (buf[1] == 0x17)
    {
        // LOGD("parse data success");
        if (len < 20)
        {
            LOGD("invalid data\n");
            return -3;
        }

        int data_len = (buf[18] | buf[19] << 8);
        LOGD("modbus msg len = %d\n", data_len);
        char *data = serial_recv_data;
        if (data_len > 7000)
        {
            LOGD("data_len too long [%d]", data_len);
            return -2;
        }
        // LOGD("data_len = %d", data_len);    
        memcpy(data, buf + 20, data_len);
        // LOGD("data = [%s]", data);
        // printf("start parse\n");
        unsigned char rdata[8192] = {0};
        int rdata_len = sizeof(rdata);
        int data_index = 4;
        data_index+=4+2+2;//Magic+channel+cmd_len
        unsigned char *rdata_pos = rdata + data_index;
        int rdata_len_remain = rdata_len - data_index;
        int res_len = 4;
        memset(&s_wind_connect_up.cur_header, 0, sizeof(WindProUpdateHeader));
        int ret = wind_protocol_wrap_up_parse_header(data, data_len, &s_wind_connect_up.cur_header);
        // LOGD("parse header ret = %d cmdType [%x]", ret, s_wind_connect_up.cur_header.cmdType);
        if (ret >= 0)
        {
            switch (s_wind_connect_up.cur_header.cmdType)
            {
            case WIND_PRO_UP_CMD_VERSION:
                MODBUS_LOG_INFO("request version");
                res_len = wind_serial_operate_version(rdata_pos, rdata_len_remain, &s_wind_connect_up.cur_header);
                break;
            case WIND_PRO_UP_CMD_OPEN_USB_STREAM:
                MODBUS_LOG_INFO("open usb stream");
                door_deinit();
                ret = start_usb_connect();
                res_len = wind_protocol_wrap_up_gen_response_usb_stream((char *)rdata_pos, rdata_len_remain, ret >= 0 ? 0 : 1, &s_wind_connect_up.cur_header);
                break;
            case WIND_PRO_UP_CMD_CLOSE_USB_STREAM:
                MODBUS_LOG_INFO("close usb stream");
                stop_usb_connect();
                door_init();
                res_len = wind_protocol_wrap_up_gen_response_usb_stream((char *)rdata_pos, rdata_len_remain, 0, &s_wind_connect_up.cur_header);
                break;
            case WIND_PRO_UP_CMD_QUERY_SYSTEM:
                LOGD("request query system\n");
                res_len = wind_serial_system_event(rdata_pos, rdata_len_remain, &s_wind_connect_up.cur_header);
                break;
            case WIND_PRO_UP_CMD_QUERY_CONFIG:
                MODBUS_LOG_INFO("request query config");
                res_len = wind_serial_query_config(rdata_pos, rdata_len_remain, &s_wind_connect_up.cur_header);
                break;
            case WIND_PRO_UP_CMD_SET_CONFIG:
                MODBUS_LOG_INFO("request set config");
                res_len = wind_serial_set_config(rdata_pos, rdata_len_remain, data, data_len, &s_wind_connect_up.cur_header);
                break;
            case WIND_PRO_UP_CMD_QUERY_PHOTO:
                LOGD("request query photo");
                res_len = handle_trans_door_photo_request(rdata_pos, rdata_len_remain, data, data_len);
                break;
            case WIND_PRO_UP_CMD_GET_SELF_CHECK_RESULT:
                MODBUS_LOG_INFO("request get self check result");  
                res_len = wind_serial_query_self_check_result(rdata_pos, rdata_len_remain, &s_wind_connect_up.cur_header);
                break;
            case WIND_PRO_UP_CMD_REBOOT:
                MODBUS_LOG_INFO("request reboot");
                res_len = wind_serial_request_reboot(rdata_pos, rdata_len_remain, &s_wind_connect_up.cur_header);
                break;
            case WIND_PRO_UP_CMD_REQUEST_CONNECT:
                MODBUS_LOG_INFO("request connect");
                res_len = wind_serial_request_connect((char *)rdata_pos, rdata_len_remain, &s_wind_connect_up.cur_header);
                break;

            /*处理OTA升级相关指令*/
            case WIND_PRO_UP_CMD_OTA:
                LOGD("request ota");
                res_len = ota_process((char *)rdata_pos, rdata_len_remain, data, data_len);
                break;
            case WIND_PRO_UP_CMD_OTA_DATA_TRANS_PACKET:
                res_len = wind_serial_request_ota_data_trans_packet((char *)rdata_pos, rdata_len_remain, data, data_len, &s_wind_connect_up.cur_header);
                break;
            case WIND_PRO_UP_CMD_OTA_DATA_END:
                res_len = wind_serial_request_ota_data_end((char *)rdata_pos, rdata_len_remain, data, data_len, &s_wind_connect_up.cur_header);
                break;

            /*拍照功能*/
            case WIND_PRO_UP_CMD_TAKE_PHOTO:
                MODBUS_LOG_INFO("request take photo");
                res_len = handle_photo_request(rdata_pos, rdata_len_remain, data, data_len);
                break;

            default:
                break;
            }
        }
        if (res_len > 8192)
        {
            LOGD("invalid res data len [%d]", res_len);
            return -4;
        }
        res_len = res_len+4+2+2;//Magic + channel + cmdlen
        // printf("end parse\n");
        rdata[0] = buf[0];
        rdata[1] = buf[1];
        rdata[3] = (res_len & 0xff);
        rdata[2] = (res_len >> 8) & 0xff;

        fill_msg_header(rdata+4, res_len-8);
        // printf("rdata\n\n");
        // printf("length %d\n", res_len + data_index);
        // for(int i = 0; i < res_len + data_index; i++)
        // {
        //     printf("%02x ", rdata[i]);
        // }
        printf("\n");
        // ret = wind_modbus_wrap_send(rdata, res_len + data_index);
        // print_timestamp("start send data");
        /*************这里更改了***************** */
        ret = wind_connect_uart_write(rdata, res_len + 4);
        /*************这里更改了***************** */
        // print_timestamp("end send data");
        // LOGD("response data len [%d] buf [%s]", ret, rdata + data_index);
        if (reboot_flag == 1)
        {
            if (system("xz -df " OTA_FILE_PATH) != 0)
            {
                LOGD("Failed to decompress file with xz");
                // // verify_result = 0;
                // goto end;
            }

            // copy_file();

            LOGD("File decompression and copy successful");

            // 赋予可执行权限
            if (chmod(DEST_PATH, 0777) != 0)
            {
                LOGD("Failed to set executable permissions on %s", DEST_PATH);
                // verify_result = 0;
                // goto end;
            }
            wind_reboot();
        }
    }
    else
    {
        // LOGD("Error cmd [%x]", buf[1]);
    }
    return 0;
}

static int wind_serial_run()
{
    unsigned char buf[8000] = {0};
    int len = sizeof(buf);
    unsigned long long s = cm_gettime_tick();
    // int ret = wind_modbus_wrap_recv(buf, len);
    // print_timestamp("read data start");
    int ret = wind_connect_uart_read(buf, len);
    // print_timestamp("read data end");
    unsigned long long s1 = cm_gettime_tick();
#if WIND_CONNECT_UP_DEBUG
    // for (int i = 0; i < ret; i++)
    // {
    //     printf("%02x ", buf[i]);
    // }
    // printf("\n");
#endif
    if (ret > 0)
    {
        usleep(10000);
        // enable_uart_send();
        //  char test_buf[10] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a};
        //  int ret = cm_uart_send_until(s_wind_connect_up.uart_fd, test_buf, 2);
        //  LOGD("send data : %x %x  ret [%d]", test_buf[0],test_buf[1], ret);

        // system("echo 'AT+COMMAND' > /dev/ttyS0");
        // printf("parse data\n");
        wind_serial_parse_data(buf, ret);
    }
    usleep(1000);
    unsigned long long s2 = cm_gettime_tick();
    // LOGD("usage time read [%llu] parse [%llu]", s1 - s, s2 - s1);
    return ret;
}

void *wind_connect_up_run_thread(void *args)
{
    prctl(PR_SET_NAME, "wind_connect_up");

    while (!s_wind_connect_up.bexit)
    {
        enable_uart_recv();
        wind_serial_run();
        usleep(10);
    }
    return NULL;
}

int wind_connect_up_start()
{
    cm_init_list_head(&s_wind_connect_up.event_list.test_list_item);
    int ret = wind_serial_open();
    if (ret < 0)
    {
        LOGD("Error: open failed ret [ %d]\n", ret);
        return -1;
    }

    if (access("/system/etc/485_disable", F_OK) == 0)
    {
        LOGD("not create 485 thread\n");
        return -1;
    }

    ret = pthread_create(&s_wind_connect_up.pid, NULL, wind_connect_up_run_thread, NULL);
    if (ret < 0)
    {
        LOGD("Error: create thread failed ret [ %d]\n", ret);
        return -2;
    }
    LOGD("start to  wind_connect_up success");
    return 0;
}

int wind_connect_up_stop()
{
    LOGD("start to stop wind_connect_up");
    s_wind_connect_up.bexit = 1;
    pthread_join(s_wind_connect_up.pid, NULL);
    wind_serial_close();
    return 0;
}

int wind_connect_up_door_open(unsigned long long count, float angle)
{
    WindSystemEvent *event = (WindSystemEvent *)malloc(sizeof(WindSystemEvent));
    if (event == NULL) // Check if malloc was successful
    {
        LOGD("Error: Memory allocation failed for WindSystemEvent");
        return -1; // Return an error code
    }
    event->info.cmd = 1;
    event->info.open_count = count;
    event->info.angle = angle;
    LOGD("add event door open count [%llu], open angle [%f]", count, angle);
    cm_list_add_tail(&event->test_list_item, &s_wind_connect_up.event_list.test_list_item);
    return 0;
}

int wind_connect_up_door_close(DoorInfoItem *item)
{
    WindSystemEvent *event = (WindSystemEvent *)malloc(sizeof(WindSystemEvent));
    if (event == NULL) // Check if malloc was successful
    {
        LOGD("Error: Memory allocation failed for WindSystemEvent");
        return -1; // Return an error code
    }
    event->info.cmd = 2;
    event->info.open_count = item->open_count;
    event->info.angle = item->image_angle;
    LOGD("add event door close count [%llu] angle [%d]", item->open_count, event->info.angle);
    cm_list_add_tail(&event->test_list_item, &s_wind_connect_up.event_list.test_list_item);
    return 0;
}

#include "cm_video_interface.h"
static CMVideoContext *video_ctx = NULL;
extern const CMVideoImpl cm_video_impl_t23;
#define TAKE_PHOTO_DIR "/tmp/take_photo"
#define TAKE_PHOTO_PATH TAKE_PHOTO_DIR "/photo.jpg"
int wind_serial_take_photo(unsigned char *res, int len, char *data, int data_len, WindProUpdateHeader *header)
{
    /*拍照*/
    printf("request take photo\n");
    static char *image_path = TAKE_PHOTO_PATH;
    static char take_photo_flag = 0;
    WindProUpdateQueryPhoto query_photo;
    memset(&query_photo, 0, sizeof(WindProUpdateQueryPhoto));
    wind_protocol_wrap_up_parse_header(data, data_len, &query_photo.header);
    wind_protocol_wrap_up_parse_request_photo(data, data_len, &query_photo);
    // char image_path[128] = {0};
    // snprintf(image_path, sizeof(image_path), "/tmp/data/%d/img-%d-%d.jpg",
    //          query_photo.open_count, query_photo.open_count, query_photo.angle);

    // 检查并创建目录
    if (access(TAKE_PHOTO_DIR, F_OK) != 0)
    {
        LOGD("Creating photo directory: %s", TAKE_PHOTO_DIR);
        // 使用mkdir创建目录，权限设置为0777
        if (mkdir(TAKE_PHOTO_DIR, 0777) != 0)
        {
            LOGD("Failed to create directory %s: %s", TAKE_PHOTO_DIR, strerror(errno));
            query_photo.header.result = 5;
            return wind_protocol_wrap_up_take_photo_response((char *)res, len, &query_photo, header);
        }
        // 确保目录权限正确
        chmod(TAKE_PHOTO_DIR, 0777);
        LOGD("Photo directory created successfully");
    }

    if (take_photo_flag == 0)
    {
        cm_video_take_photo_save_to_file(image_path);
        take_photo_flag = 1;
        struct stat st;
        if (stat(image_path, &st) == 0)
        {
            LOGD("New photo captured - Size: %lu bytes (%.2f KB)",
                 st.st_size,
                 st.st_size / 1024.0);
        }
        else
        {
            LOGD("Failed to get photo file size: %s", strerror(errno));
        }
    }

    // char image_path = "/tmp/a.jpg";

    if (access(image_path, F_OK) == 0)
    {
        size_t image_size = cm_file_size(image_path);
        if (query_photo.offset > image_size)
        {
            LOGD("Error: offset [%llu] > image size[%u]", query_photo.offset, image_size);
            query_photo.header.result = 2;
            query_photo.size = image_size;
            query_photo.data_len = 0;
            take_photo_flag = 0;
            return wind_protocol_wrap_up_take_photo_response((char *)res, len, &query_photo, header);
        }
        query_photo.header.result = 0;
        query_photo.size = image_size;
        LOGD("will read file [%s] offset [%llu], length [%llu]", image_path, query_photo.offset, query_photo.length);
        FILE *fp = fopen(image_path, "rb");
        if (fp)
        {
            fseek(fp, query_photo.offset, SEEK_SET);
            query_photo.data_len = fread(query_photo.data, 1, query_photo.length, fp);
            fclose(fp);
            if (query_photo.data_len < 0)
            {
                LOGD("Error: read image fail [%d]", errno);
                query_photo.header.result = 3;
                query_photo.data_len = 0;
                take_photo_flag = 0;
                return wind_protocol_wrap_up_take_photo_response((char *)res, len, &query_photo, header);
            }
            LOGD("read image success len [%d]", query_photo.data_len);

            /* 判断是否读取完，然后删除图片 */
            if (query_photo.offset + query_photo.data_len >= image_size)
            {
                /*读取完毕， 删除图片*/
                LOGD("read all image frame end\n\n");
                delete_img(image_path);
                take_photo_flag = 0;
            }
            else
            {
                /*未读取完*/
            }
            /* 判断是否读取完，然后删除图片 */

            if (query_photo.data_len > 8)
            {
                LOGD("read image data [%x %x %x %x] [%x %x %x %x]",
                     query_photo.data[0], query_photo.data[1], query_photo.data[2], query_photo.data[3],
                     query_photo.data[query_photo.data_len - 4], query_photo.data[query_photo.data_len - 3],
                     query_photo.data[query_photo.data_len - 2], query_photo.data[query_photo.data_len - 1]);
            }
            else
            {
                for (int i = 0; i < query_photo.data_len; i++)
                {
                    printf("%02x ", query_photo.data[i]);
                }
                printf("\n");
            }
            LOGD("------ [%d]---------", query_photo.data_len);
            return wind_protocol_wrap_up_take_photo_response((char *)res, len, &query_photo, header);
        }
        else
        {
            LOGD("Error: open image fail [%d]", errno);
            query_photo.header.result = 4;
            query_photo.data_len = 0;
            return wind_protocol_wrap_up_take_photo_response((char *)res, len, &query_photo, header);
        }
    }
    else
    {
        LOGD("Error: image [%s] not exist", image_path);
        query_photo.header.result = 1;
        query_photo.size = 0;
        query_photo.data_len = 0;
        take_photo_flag = 0;
        return wind_protocol_wrap_up_take_photo_response((char *)res, len, &query_photo, header);
    }
}