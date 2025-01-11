#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include "CommonMsgTable.h"
#include "MsgDispatcher.h"
#include "cm_common.h"
#define MSG_BUF_SIZE 1024

typedef struct MsgNode {
    uint8_t *msg;
    uint8_t len;
    struct MsgNode *next;
} MsgNode;

// 队列结构
typedef struct ThreadSafeQueue {
    MsgNode *head;
    MsgNode *tail; 
    pthread_mutex_t mutex;
} ThreadSafeQueue;



static ThreadSafeQueue send_queue;
static ThreadSafeQueue recv_queue;
static uint8_t MsgRecvBuf[MSG_BUF_SIZE];
// static MsgNode *send_head = NULL;
// static MsgNode *recv_head = NULL;
static DataTransInterface data_trans_interface;





// 初始化队列
void init_queue(ThreadSafeQueue *q) {
    q->head = NULL;
    q->tail = NULL;
    pthread_mutex_init(&q->mutex, NULL);
}

// 添加消息到队列（追加到尾部）
int add_msg_to_queue(ThreadSafeQueue *q, const uint8_t *msg, uint8_t len)
{
    MsgNode *new_node = (MsgNode *)malloc(sizeof(MsgNode));
    if (new_node == NULL) {
        perror("Failed to allocate memory for new node");
        return -1;
    }

    new_node->msg = (uint8_t *)malloc(len);
    if (new_node->msg == NULL) {
        perror("Failed to allocate memory for message");
        free(new_node);
        return -1;
    }

    memcpy(new_node->msg, msg, len);
    new_node->len = len;
    new_node->next = NULL;

    pthread_mutex_lock(&q->mutex);
    if (q->tail != NULL) {
        q->tail->next = new_node;
        q->tail = new_node;
    } else {
        // 如果队列为空，则新节点既是头也是尾
        q->head = q->tail = new_node;
    }
    pthread_mutex_unlock(&q->mutex);

    return 0;
}

// 移除队列中的消息并返回节点（从头部移除）
MsgNode* remove_msg_from_queue(ThreadSafeQueue *q)
{
    pthread_mutex_lock(&q->mutex);
    if (q->head == NULL) {
        pthread_mutex_unlock(&q->mutex);
        return NULL; // 表示失败（空列表）
    }

    MsgNode *node = q->head;
    q->head = q->head->next;
    if (q->head == NULL) {
        // 如果队列变为空，则更新尾指针
        q->tail = NULL;
    }
    pthread_mutex_unlock(&q->mutex);

    return node;
}

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
uint8_t send_msg_resp(uint8_t *msg, uint8_t len)
{
    return add_msg_to_queue(&send_queue, msg, len) == 0 ? 0 : 1; // 0 for success, 1 for failure
}
static int send_msg_pre(uint8_t *req, int req_length)
{
    uint16_t crc = crc16(req, req_length);

    /* According to the MODBUS specs (p. 14), the low order byte of the CRC comes
     * first in the RTU message */
    req[req_length++] = crc & 0x00FF;
    req[req_length++] = crc >> 8;

    return req_length;
}
static uint8_t send_msg_low_level(uint8_t *msg, uint8_t len)
{
    // send_msg_pre(msg, len);
    return data_trans_interface.send_data(msg, len);
}

static uint8_t recv_msg_low_level(uint8_t *msg, uint8_t len)
{
    return data_trans_interface.recv_data(msg, len);
}

static int msg_poll(uint8_t *data, uint32_t data_len)
{
    int step = 1;
    int pos = 0;
    int length = 0;
    int received_len = 0;

    while (1)
    {
        switch (step)
        {
        case 1:
            received_len = recv_msg_low_level(data + pos, 2);
            if (received_len != 2 || !(data[pos] == 0xAA && data[pos + 1] == 0x5A))
            {
                break;
            }
            LOGD("Received frame header\n");
            step = 2;
            pos += 2;
            break;

        case 2:
            received_len = recv_msg_low_level(data + pos, 2);
            if (received_len != 2 || data[pos] != SLAVE_ADDR)
            {
                step = 1;
                pos = 0;
                break;
            }
            LOGD("Received slave address\n");
            step = 3;
            pos += 2;
            break;

        case 3:
            received_len = recv_msg_low_level(data + pos, 4);
            if (received_len != 4)
            {
                step = 1;
                pos = 0;
                break;
            }
            pos += received_len;
            if (pos >= 8)
            {
                length = (data[pos - 1] << 24) |
                         (data[pos - 2] << 16) |
                         (data[pos - 3] << 8) |
                         data[pos - 4];
                length += 2; // Include CRC
                LOGD("Data length [%d]\n", length);
                if (length > 8000)
                {
                    step = 1;
                    pos = 0;
                    LOGD("Invalid data length [%d]\n", length);
                    break;
                }
                else
                {
                    step = 4;
                }
            }
            break;

        case 4:
            received_len = recv_msg_low_level(data + pos, length);
            if (received_len != length)
            {
                step = 1;
                pos = 0;
                LOGD("received_len : %d\n", received_len);
                LOGD("length not equal\n");
                break;
            }
            pos += received_len;
            if (pos == length + 8)
            {
                // LOGD("dara[pos-1] : %d\n",data[pos - 1]);
                // LOGD("dara[pos-2] : %d\n",data[pos - 2]);
                // uint16_t calculated_crc = crc16(data, pos - 2);
                // uint16_t received_crc = (data[pos - 1] << 8) | data[pos - 2];
                // if (calculated_crc == received_crc)
                // {
                //     return pos;
                // }
                // else
                // {
                //     step = 1;
                //     pos = 0;
                //     LOGD("CRC error calculated_crc : %d , received_crc : %d\n",calculated_crc,received_crc);
                //     return -1;
                // }
                return pos;
            }
            break;

        default:
            step = 1;
            break;
        }
    }
    return 0;
}

static void *msg_send_thread(void *arg)
{
    LOGD("msg_send_thread ready");
        
        uint8_t *msg;
        uint8_t len;
        MsgNode *node = NULL;
        while (1)
        {
            node = remove_msg_from_queue(&send_queue);
            if (node!=NULL)
            {
                if (MSG_DISPATHER_DEBUG_EN)
                {
                    for (int i = 0; i < node->len; i++)
                    {
                        printf("%02X ", node->msg[i]); // 打印每个字节的十六进制表示
                    }
                    printf("\n"); // 换行以便于输出格式
                }
                send_msg_low_level(node->msg, node->len);
                free(node->msg);
                free(node);
                node=NULL;
            }
        }
    return NULL;
}

static void *msg_poll_thread(void *arg)
{
    LOGD("msg_poll_thread ready\n");
    while (1)
    {
        int received_len = msg_poll(MsgRecvBuf, sizeof(MsgRecvBuf));
        if (MSG_DISPATHER_DEBUG_EN)
        {
            for (int i = 0; i < received_len; i++)
            {
                printf("%02X ", MsgRecvBuf[i]); // 打印每个字节的十六进制表示
            }
            printf("\n"); // 换行以便于输出格式
        }
        if (received_len > 0)
        {
            LOGD("add msg to queue\n");
            add_msg_to_queue(&recv_queue, MsgRecvBuf, received_len);
            // Implement your callback logic here
        }
        usleep(1000);
    }
    return NULL;
}

static void process_message(uint8_t *msg, uint8_t len)
{
    for (int i = 0; i < SID_NUM; i++)
    {
        if (MsgServiceList[i].sid == msg[3])
        {
            LOGD("SID : %d", MsgServiceList[i].sid);
            MsgServiceList[i].msg_process_callback(msg, len);
            break;
        }
    }
}

static void *msg_process_thread(void *arg)
{
    LOGD("msg_process_thread ready");
    MsgNode *msg_node = NULL;
    while (1)
    {
        uint8_t *msg;
        uint8_t len;
        msg_node = remove_msg_from_queue(&recv_queue);
        if(msg_node!=NULL)
        {
            LOGD("get msg node\n");
            process_message(msg_node->msg, msg_node->len);
            free(msg_node->msg);
            free(msg);
        }
    }
    return NULL;
}

void MsgDispatcherInit(DataTransInterface *interface)
{
    if (interface == NULL)
    {
        LOGD("Error: Interface initialization failed\n");
        return;
    }

    // send_head = NULL;
    // recv_head = NULL;
    data_trans_interface = *interface;

    if (data_trans_interface.init() != 0)
    {
        LOGD("Error: Low-level data transport initialization failed\n");
        return;
    }
    init_queue(&send_queue);
    init_queue(&recv_queue);
    pthread_t send_tid, poll_tid, process_tid;
    pthread_create(&send_tid, NULL, msg_send_thread, NULL);
    pthread_create(&poll_tid, NULL, msg_poll_thread, NULL);
    pthread_create(&process_tid, NULL, msg_process_thread, NULL);

    // Optionally, detach threads if they are daemon-like and should run indefinitely
    pthread_detach(send_tid);
    pthread_detach(poll_tid);
    pthread_detach(process_tid);
}

void Cam485ProtocolInit()
{
    extern DataTransInterface uart_interface;
    MsgDispatcherInit(&uart_interface);
}
