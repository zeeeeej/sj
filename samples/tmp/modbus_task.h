#ifndef MODBUS_DISPATCHER_H
#define MODBUS_DISPATCHER_H

#include <mqueue.h>

#include "ota_process.h"
#include "mem_pool.h"

// 常量定义
#define MAX_COMMANDS 32
#define MAX_QUEUE_SIZE 10
#define MAX_MESSAGE_SIZE 1024

// 错误码定义
#define MODBUS_OK 0
#define MODBUS_ERROR_INVALID_PARAM -1
#define MODBUS_ERROR_QUEUE_FULL -2
#define MODBUS_ERROR_NO_MEMORY -3
#define MODBUS_ERROR_NOT_FOUND -4


// 内存池配置
#define MESSAGE_BLOCK_SIZE 1024
#define MESSAGE_BLOCK_COUNT 32

// 类型定义
typedef void (*ModuleInitFuncPtr)(void);
typedef struct {
    char *data;
    int length;
    void (*free_ptr)(void *priv, void *data);
}QueueMessage;
// 函数声明
int init_modbus_dispatcher(void);
void deinit_modbus_dispatcher(void);
int dispatch_modbus_command(int func_code, const void *data, size_t length);
int dispatch_modbus_message(mqd_t mq, const void *data, size_t length, int func_code);
int fetch_modbus_send_data(void *buffer, size_t buf_size, size_t *recv_len);
void register_message_queues(mqd_t* recv_que, mqd_t* send_que, int func_code, ModuleInitFuncPtr init_func);
int unregister_message_queue(int func_code);
int get_queue_status(mqd_t mq);

#endif // MODBUS_DISPATCHER_H