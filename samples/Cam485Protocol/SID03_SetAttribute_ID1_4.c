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

static pthread_mutex_t uart_mutex = PTHREAD_MUTEX_INITIALIZER;

static int AttributeSetResp(uint8_t id,uint8_t result);
static int setRs485Baudrate(uint32_t baudrate);

void initRs485(uint32_t newBaudrate)
{
    printf("Timer callback executed2 baudrate: %u\n", newBaudrate);
    int uart_fd = get_RS485uart_fd();

    pthread_mutex_lock(&uart_mutex);
    /*关闭串口*/
    cm_uart_close(uart_fd);
    //usleep(1000*10);

    /*测试设置新波特率能不能成功*/
    uart_fd = cm_uart_open("/dev/ttyS0");
    /*打开文件成功*/
    if (uart_fd > 0)
    {
        LOGD("cm_uart_open ok\n");
        
        int ret = cm_uart_init(uart_fd, newBaudrate, 0, 8, 1, 'n');// 0 1
        /*这一步判断设置波特率是不是成功*/
        if (ret < 0)
        {
            /*失败了，重新设置会原来的波特率*/
            log_e("uart init failed");
            /*关闭串口*/
            cm_uart_close(uart_fd);
            //usleep(1000*10);

            /*重新设置会原来的波特率*/
            uart_fd = cm_uart_open("/dev/ttyS0");
            uint32_t Baudrate = Get_Rs485Baudrate();
            LOGD("Baudrate=%u\n",Baudrate);
            cm_uart_init(uart_fd, Baudrate, 0, 8, 1, 'n');
            //usleep(1000*10);
            /*回复海大，设置失败*/
            AttributeSetResp(4,1);
            pthread_mutex_unlock(&uart_mutex);
            return;
        }

        /*设置成功了*/
        cm_uart_close(uart_fd);
        //usleep(1000*10);

        /*这里还是要用原来的波特率回复一下海大，设置成功了*/
        uart_fd = cm_uart_open("/dev/ttyS0");
        uint32_t Baudrate = Get_Rs485Baudrate();
        LOGD("Baudrate=%u\n",Baudrate);
        cm_uart_init(uart_fd, Baudrate, 0, 8, 1, 'n');
        //usleep(1000*10);
        /*回复海大，设置成功了*/
        AttributeSetResp(4,0);

        pthread_mutex_unlock(&uart_mutex);
        /*2秒后，设置为新的波特率*/
        usleep(1000*1000*2);
        pthread_mutex_lock(&uart_mutex);
        cm_uart_close(uart_fd);
        //usleep(1000*10);

        /*设置为新的波特率*/
        LOGD("rs485 set newBaudrate %u\n",newBaudrate);
        uart_fd = cm_uart_open("/dev/ttyS0");
        cm_uart_init(uart_fd, newBaudrate, 0, 8, 1, 'n');
        pthread_mutex_unlock(&uart_mutex);

        /*保存句柄和波特率*/
        set_RS485uart_fd(uart_fd);
        setRs485Baudrate(newBaudrate);
        LOGD("cm_uart_init ok\n");
    }
}


// 回调函数
void *timer_callback(void *args) 
{
    uint32_t baudrate = *(uint32_t *)args;
    printf("Timer callback executed1 baudrate: %u\n", baudrate);
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
    RS485_reinit(baudrate);
    return 0;
}
