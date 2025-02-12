/*SID03_01-04*/

#include "cm_common.h"
#include "stdbool.h"
#include "SID03_SetAttribute.h"
#include "Cam485ProtocolCommon.h"
#include "ImageInfoList.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <pthread.h>

static int AttributeSetResp(uint8_t id,uint8_t result);


// 被调用的函数
void signal_handler(int signum) {
    static int count = 0;
    printf("Function xxx is executed for the %d time.\n", ++count);
}

int RS485_reinit0(uint32_t Rs485Baudrate) {
    // 注册信号处理函数
    if (signal(SIGALRM, signal_handler) == SIG_ERR) {
        perror("signal");
        return 1;
    }

    // 设置定时器
    struct itimerval value;
    value.it_value.tv_sec = 2;  // 首次触发时间，2秒
    value.it_value.tv_usec = 0;
    value.it_interval.tv_sec = 0;  // 后续触发间隔时间，0秒  就出发一次
    value.it_interval.tv_usec = 0;

    if (setitimer(ITIMER_REAL, &value, NULL) == -1) {
        perror("setitimer");
        return 1;
    }

    return 0;
}

void initRs485(uint32_t newBaudrate)
{
    printf("Timer callback executed2 baudrate: %u\n", newBaudrate);
    int uart_fd = get_RS485uart_fd();

    cm_uart_close(uart_fd);

    usleep(1000*10);

    int ret = cm_uart_open("/dev/ttyS0");
    if (ret > 0)
    {
        LOGD("cm_uart_open ok\n");
        uart_fd = ret;
        ret = cm_uart_init(ret, newBaudrate, 0, 8, 1, 'n');
        if (ret < 0)
        {
            log_e("uart init failed");
            cm_uart_close(uart_fd);
            usleep(1000*10);
            int ret = cm_uart_open("/dev/ttyS0");
            uart_fd = ret;
            uint32_t Baudrate_old = get_RS485OldBaudrate();
            cm_uart_init(ret, Baudrate_old, 0, 8, 1, 'n');
            usleep(1000*10);
            AttributeSetResp(4,1);
            return;
        }
        set_RS485OldBaudrate(newBaudrate);
        usleep(1000*10);
        AttributeSetResp(4,0);
        LOGD("cm_uart_init ok\n");
    }
}

// 回调函数
void *timer_callback(void *args) 
{
    uint32_t baudrate = *(uint32_t *)args;
    printf("Timer callback executed1 baudrate: %u\n", baudrate);
    usleep(1000*1000*2);
    initRs485(baudrate);
    return NULL;
}


int RS485_reinit(uint32_t Rs485Baudrate) 
{
    pthread_t timerThread;
    LOGD("Rs485Baudrate = %u\n",Rs485Baudrate);

    if (pthread_create(&timerThread, NULL, timer_callback, (void*)(&Rs485Baudrate))!= 0) {
        perror("pthread_create");
        return 1;
    }
    pthread_detach(timerThread);
    LOGD("Timer start\n");
    usleep(1000*100);//延时目的是让回调函数能拿到参数后再释放资源

    return 0;
}


static int BuildMsgHeader(uint8_t *msg_buf, uint32_t msg_dlc)
{
    int index = 0;
    msg_buf[index++] = 0xAA;
    msg_buf[index++] = 0x5A;
    msg_buf[index++] = SLAVE_ADDR;
    msg_buf[index++] = 0x03;

    /*填充长度字段*/
    msg_buf[index++] = (uint8_t)(msg_dlc & 0xFF);         // Low byte
    msg_buf[index++] = (uint8_t)((msg_dlc >> 8) & 0xFF);  // 2nd byte
    msg_buf[index++] = (uint8_t)((msg_dlc >> 16) & 0xFF); // 3rd byte
    msg_buf[index++] = (uint8_t)((msg_dlc >> 24) & 0xFF); // High byte
}

static int AttributeSetResp(uint8_t id,uint8_t result)
{
    uint8_t resp_buf[10+2];
    memset(resp_buf, 0, sizeof(resp_buf));
    BuildMsgHeader(resp_buf, 2);
    resp_buf[8] = id;
    resp_buf[9] = result;
    send_msg_resp(resp_buf, 10+2);
    return 0;
}

static int setSloveAddress(uint8_t slave_address)
{
    uint8_t ret = 0;
    Set_g_slave_address(slave_address);
    return ret;
}

static int setFirmwareVersion(uint32_t firmwareVersion)
{
    uint8_t ret = 0;
    Set_g_firmwareVersion(firmwareVersion);
    return ret;
}

static int setCameraNum(uint32_t camera_num)
{
    uint8_t ret = 0;
    Set_g_CameraNum(camera_num);
    return ret;
}

static int setRs485Baudrate(uint32_t baudrate)
{
    uint8_t ret = 0;
    Set_g_Rs485Baudrate(baudrate);
    return ret;
}

int Handle_SID03_Attribute_SloveAddress(uint8_t *msg_buf, uint32_t msg_dlc)
{
    uint8_t result = 0;
    uint8_t slave_address = msg_buf[9];
    LOGD("slave_address = %d\n",slave_address);
    result = setSloveAddress(slave_address);
    AttributeSetResp(1,result);
    return 0;
}

int Handle_SID03_Attribute_FirmwareVersion(uint8_t *msg_buf, uint32_t msg_dlc)
{
    uint8_t result = 0;
    uint32_t firmware_version = ((msg_buf[9])|(msg_buf[10]<<8)|(msg_buf[11]<<16)|(msg_buf[12]<<24));
    LOGD("firmware_version = %d\n",firmware_version);
    result = setFirmwareVersion(firmware_version);
    if(0 != result)
    {
        LOGD("ERROR:setFirmwareVersion\n");
    }
    AttributeSetResp(2,result);
    return 0;
}

int Handle_SID03_Attribute_CameraNum(uint8_t *msg_buf, uint32_t msg_dlc)
{
    uint8_t result = 0;
    uint32_t camera_num = ((msg_buf[9])|(msg_buf[10]<<8)|(msg_buf[11]<<16)|(msg_buf[12]<<24));
    LOGD("camera_num = %d\n",camera_num);
    result = setCameraNum(camera_num);
    if(0 != result)
    {
        LOGD("ERROR:setCameraNum\n");
    }
    AttributeSetResp(3,result);
    return 0;
}

int Handle_SID03_Attribute_Baudrate(uint8_t *msg_buf, uint32_t msg_dlc)
{
    uint8_t result = 0;
    uint32_t baudrate = ((msg_buf[9])|(msg_buf[10]<<8)|(msg_buf[11]<<16)|(msg_buf[12]<<24));
    LOGD("baudrate = %d\n",baudrate);
    result = setRs485Baudrate(baudrate);
    if(0 != result)
    {
        LOGD("ERROR:setRs485Baudrate\n");
    }
    //AttributeSetResp(4,result);
    RS485_reinit(baudrate);
    return 0;
}
