#include "MessageDispatcher.h"
#include "MessageDispaterPort.h"
#include "cJSON.h"
#include "ota_process.h"
#include "take_photo.h"

#include "stdbool.h"
#include "stdint.h"
#include "string.h"
#include "stdio.h"
#include "pthread.h"
typedef struct
{
    uint8_t magic[4];  // 魔数
    uint16_t channel;  // 通道号
    uint16_t cmd_len;  // 命令长度
    uint16_t data_len; // 数据长度
} MessageHeader;

typedef struct
{
    uint64_t timestamp;
    uint32_t seqNum;
    uint32_t cmdType;
    int result;
} MessageSubHeader;

static MessageDispatcher_Interface interface;
void *interface_ctx;
#define RECV_BUF_SIZE 5000
#define SEND_BUF_SIZE 5000
static uint8_t MessageDispatcher_recv_buf[RECV_BUF_SIZE];
static uint8_t MessageDispatcher_send_buf[SEND_BUF_SIZE];
static MessageHeader header;
static MessageSubHeader sub_header;
static pthread_t MessageDispatcher_thread;
static int MessageDispatcher_recv(uint8_t *buffer, int buffer_size);
static int MessageDispatcher_send(const uint8_t *message, int length);
static int MessageDispatcher_parse_message(const uint8_t *message, int length);
static int MessageDispatcher_process_message(const uint8_t *message, int length, uint8_t *response, int response_size);
void message_dispatcher_thread(void *ctx);
static void MessageDispatcher_Init()
{
    register_interface(&interface);
    interface.init();
}

static int MessageDispatcher_recv(uint8_t *buffer, int buffer_size)
{
    return interface.receive_message(buffer, buffer_size);
}

static int MessageDispatcher_send(const uint8_t *message, int length)
{
    return interface.send_message(message, length);
}

static int MessageDispatcher_fill_send_sub_header(uint8_t *buf, int length)
{
    if (!buf || length <= 0)
    {
        LOGD("Invalid parameters");
        return -1;
    }
    buf[0] = 0xFD;
    buf[1] = 0xFC;
    buf[2] = 0xFB;
    buf[3] = 0xFA;

    buf[4] = 0x64; // 101 的低字节 (0x65)
    buf[5] = 0x00; // 101 的高字节 (0x00)

    buf[6] = length & 0xff;
    buf[7] = (length >> 8) & 0xff;
}

static int MessageDispatcher_fill_message_header(uint8_t *buf, int length)
{
    if (!buf || length <= 0)
    {
        LOGD("Invalid parameters");
        return -1;
    }
    int total_length = length + MESSAGE_SEND_SUB_HEADER_LENGTH;
    buf[0] = DEVICE_ADDRESS;
    buf[1] = MESSAGE_DEVICE_FUNC_CODE;

    buf[2] = (total_length >> 8) & 0xff;
    buf[3] = total_length & 0xff;
    MessageDispatcher_fill_send_sub_header(buf + MESSAGE_SEND_HEADER_LENGTH, length);
    return length + MESSAGE_SEND_HEADER_LENGTH + MESSAGE_SEND_SUB_HEADER_LENGTH;
}

/**
 * @brief 解析消息头部
 * @param buf 输入数据buffer
 * @param len buffer长度
 * @param header 解析后的头部结构
 * @return 成功返回0,失败返回负数
 */
static int parse_message_header(const uint8_t *buf, int len, MessageHeader *header)
{
    if (!buf || !header || len < sizeof(MessageHeader))
    {
        LOGD("Invalid parameters or buffer too small");
        return -1;
    }

    // 检查魔数
    if (buf[1] != 0x17)
    {
        LOGD("Invalid magic number: 0x%02x", buf[1]);
        return -2;
    }

    uint8_t *sub_header_ptr = buf + MESSAGE_RECV_HEADER_LENGTH;
    // 解析头部字段
//     header->channel = (buf[4] | (buf[5] << 8));
//     header->cmd_len = (buf[6] | (buf[7] << 8));
#if MESSAGE_DISPATCHER_DEBUG
    printf("sub_header_ptr:\n");
    for (int i = 0; i < 6; i++) // 调整循环以打印正确的范围
    {
        printf("%x ", sub_header_ptr[i]);
    }
    printf("\n");
#endif
    header->cmd_len = (sub_header_ptr[6] | (sub_header_ptr[7] << 8));

#if MESSAGE_DISPATCHER_DEBUG
    // printf("header->channel [%d]\n", header->channel);
    printf("header->cmd_len [%d]\n", header->cmd_len);
    // printf("header->data_len [%d]\n", header->data_len);
#endif
    //     // 验证数据长度
    //     if (header->data_len > 7000)
    //     {
    //         LOGD("Data length too long: %d", header->data_len);
    //         return -3;
    //     }

    return 0;
}

/**
 * @brief 解析子消息头部
 * @param data JSON格式的数据
 * @param len 数据长度
 * @param header 解析后的子消息头结构
 * @return 0:成功 -1:参数错误 -2:JSON解析错误 -3:缺少必要字段
 */
static int parse_message_sub_header(const uint8_t *data, int len, MessageSubHeader *header)
{
    // 参数检查
    if (!data || !header || len <= 0)
    {
        LOGD("Invalid parameters");
        return -1;
    }

    // 初始化header
    memset(header, 0, sizeof(MessageSubHeader));

    // 解析JSON
    cJSON *root = cJSON_ParseWithLength((const char *)data, len);
    if (!root)
    {
        LOGD("Failed to parse JSON data: %s", cJSON_GetErrorPtr());
        return -2;
    }

    // 使用标志位跟踪必要字段是否存在
    bool has_timestamp = false;
    bool has_seqNum = false;
    bool has_cmdType = false;

    // 解析timestamp
    cJSON *timestamp = cJSON_GetObjectItem(root, "timestamp");
    if (timestamp && cJSON_IsNumber(timestamp))
    {
        header->timestamp = (uint64_t)cJSON_GetNumberValue(timestamp);
        has_timestamp = true;
    }

    // 解析seqNum
    cJSON *seqNum = cJSON_GetObjectItem(root, "seqNum");
    if (seqNum && cJSON_IsNumber(seqNum))
    {
        header->seqNum = (uint32_t)cJSON_GetNumberValue(seqNum);
        has_seqNum = true;
    }

    // 解析cmdType
    cJSON *cmdType = cJSON_GetObjectItem(root, "cmdType");
    if (cmdType && cJSON_IsNumber(cmdType))
    {
        header->cmdType = (uint32_t)cJSON_GetNumberValue(cmdType);
        has_cmdType = true;
    }

    // 清理JSON对象
    cJSON_Delete(root);

    // 验证所有必要字段是否都存在
    if (!has_timestamp || !has_seqNum || !has_cmdType)
    {
        LOGD("Missing required fields: timestamp(%d) seqNum(%d) cmdType(%d)",
             has_timestamp, has_seqNum, has_cmdType);
        return -3;
    }

    LOGD("Parsed sub header - timestamp: %llu, seqNum: %u, cmdType: 0x%x",
         header->timestamp, header->seqNum, header->cmdType);

    return 0;
}
static int MessageDispatcher_parse_message(const uint8_t *message, int length)
{

    if (!message || length <= 0)
    {
        LOGD("Invalid message parameters");
        return -1;
    }
    uint8_t *recv_header_ptr = message;
    uint8_t *recv_sub_header_ptr = message + MESSAGE_RECV_SUB_HEADER_LENGTH + MESSAGE_RECV_HEADER_LENGTH;

    int ret = parse_message_header((const uint8_t *)recv_header_ptr, length, &header);
    if (ret != 0)
    {
        LOGD("Failed to parse message header: %d", ret);
        return ret;
    }
#if MESSAGE_DISPATCHER_DEBUG
    for (int i = 0; i < 10; i++)
    {
        printf("%x ", recv_sub_header_ptr[i]);
    }
    printf("\n");
#endif
    ret = parse_message_sub_header(recv_sub_header_ptr, length - MESSAGE_RECV_SUB_HEADER_LENGTH, &sub_header);
    if (ret != 0)
    {
        LOGD("Failed to parse message sub header: %d", ret);
        return ret;
    }

    return 0;
}

/**
 * @brief 偏移消息指针
 * @param message 输入消息
 * @return 偏移后的指针
 */
static uint8_t *offset_recv_message_pointer(const uint8_t *message)
{
    if (!message)
    {
        LOGD("Invalid message pointer");
        return NULL;
    }
    // return (uint8_t *)message + MESSAGE_RECV_HEADER_LENGTH ;
    return (uint8_t *)message + MESSAGE_RECV_HEADER_LENGTH + MESSAGE_RECV_SUB_HEADER_LENGTH;
}

/**
 * @brief 偏移消息指针
 * @param message 输入消息
 * @return 偏移后的指针
 */
static uint8_t *offset_send_message_pointer(const uint8_t *message)
{
    if (!message)
    {
        LOGD("Invalid message pointer");
        return NULL;
    }
    return (uint8_t *)message + MESSAGE_SEND_HEADER_LENGTH + MESSAGE_SEND_SUB_HEADER_LENGTH;
}

int MessageDispatcher_process_message(const uint8_t *recv_message, int recv_length, uint8_t *response_buf, int response_buf_size)
{

    int res_len = 0;
    uint8_t *recv_buf_ptr = offset_recv_message_pointer(recv_message);
    uint8_t *response_buf_ptr = offset_send_message_pointer(response_buf);
    uint32_t cmd_len = header.cmd_len;
    printf("cmd_len [%d]\n", cmd_len);
    switch (sub_header.cmdType)
    {
    /*处理OTA升级相关指令*/
    case MESSAGE_CMD_OTA:
        res_len = ota_process(response_buf_ptr, response_buf_size, recv_buf_ptr, cmd_len);
        break;

    /*拍照功能*/
    case MESSAGE_CMD_TAKE_PHOTO:
        res_len = handle_photo_request(response_buf_ptr, response_buf_size, recv_buf_ptr, cmd_len);
        break;

    default:
        break;
    }

    return res_len;
}

int create_message_dispatcher_thread()
{
    printf("abababab\n");
    int ret = pthread_create(&MessageDispatcher_thread, NULL, message_dispatcher_thread, NULL);
    if (ret < 0)
    {
        LOGD("Error: create thread failed ret [ %d]\n", ret);
        return -2;
    }
    else
    {
        printf("Thread created successfully.\n");
    }
    return 0;
}

void message_dispatcher_thread(void *ctx)
{
    int ret = 0;
    int recv_len = 0;
    int send_len = 0;
    printf("MessageDispatcher_Init\n");
    MessageDispatcher_Init();
    usleep(10000);
    printf("MessageDispatcher_Init end\n");
    while (1)
    {
        printf("start recv\n");
        recv_len = MessageDispatcher_recv(MessageDispatcher_recv_buf, RECV_BUF_SIZE);
        if (recv_len <= 0)
        {
            LOGD("Failed to recv message: %d", recv_len);
            break;
        }
#if MESSAGE_DISPATCHER_DEBUG
        printf("recv_len [%d]\n", recv_len);
        printf("recv_buf:\n");
        for (int i = 0; i < recv_len; i++)
        {
            printf("%x ", MessageDispatcher_recv_buf[i]);
        }
        printf("\n");
#endif
        ret = MessageDispatcher_parse_message(MessageDispatcher_recv_buf, recv_len);
        if (ret != 0)
        {
            LOGD("Failed to parse message: %d", ret);
            break;
        }
        send_len = MessageDispatcher_process_message(MessageDispatcher_recv_buf, recv_len, MessageDispatcher_send_buf, SEND_BUF_SIZE);
        if (send_len <= 0)
        {
            LOGD("Failed to process message: %d", send_len);
            break;
        }
#if MESSAGE_DISPATCHER_DEBUG
        printf("send_len [%d]\n", send_len);
        printf("send_buf:\n");
        for (int i = 0; i < send_len; i++)
        {
            printf("%x ", MessageDispatcher_send_buf[i]);
        }
        printf("\n");
#endif
        send_len = MessageDispatcher_fill_message_header(MessageDispatcher_send_buf, send_len);
        if (send_len <= 0)
        {
            LOGD("Failed to fill message header: %d", send_len);
            break;
        }

#if MESSAGE_DISPATCHER_DEBUG
        printf("send_len [%d]\n", send_len);
        printf("send_buf:\n");
        for (int i = 0; i < send_len; i++)
        {
            printf("%x ", MessageDispatcher_send_buf[i]);
        }
        printf("\n");
#endif
        send_len = MessageDispatcher_send(MessageDispatcher_send_buf, send_len);
        if (send_len <= 0)
        {
            LOGD("Failed to send message: %d", send_len);
            break;
        }

        /*睡眠10ms*/
        // usleep(10000);
    }
}
