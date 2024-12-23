#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <mqueue.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


#include "modbus_task.h"

static mqd_t modbus_send_que;
typedef struct {
    int func_code;
    mqd_t recv_que;
} CommandQueue;


// 全局内存池
static MemoryPool* g_message_pool = NULL;
static pthread_mutex_t dispatcher_mutex = PTHREAD_MUTEX_INITIALIZER;
static CommandQueue command_queues[MAX_COMMANDS];
int command_count = 0;

/**
 * @brief 释放消息数据内存
 * @param priv 私有数据（未使用）
 * @param ptr 要释放的内存指针
 */
static void free_message_memory(void *priv, void *ptr) {
    if (!ptr || !g_message_pool) {
        return;
    }
    free_memory(g_message_pool, ptr);
}
/**
 * @brief 从发送队列获取消息并拷贝到缓冲区
 * @param buffer 目标缓冲区
 * @param buf_size 缓冲区大小
 * @param recv_len 实际接收到的数据长度
 * @return 成功返回0，失败返回-1
 */
int fetch_modbus_send_data(void *buffer, size_t buf_size, size_t *recv_len) {
    if (!buffer || buf_size == 0 || !recv_len) {
        fprintf(stderr, "Invalid parameters for fetching message\n");
        return -1;
    }

    // 接收消息
    QueueMessage msg;
    ssize_t bytes_received = mq_receive(modbus_send_que, (char *)&msg, sizeof(QueueMessage), NULL);
    if (bytes_received == -1) {
        fprintf(stderr, "Failed to receive message: %s\n", strerror(errno));
        return -1;
    }

    // 验证消息
    if (!msg.data || msg.length <= 0 || msg.length > buf_size) {
        fprintf(stderr, "Invalid message or buffer too small. Message length: %d, Buffer size: %zu\n", 
                msg.length, buf_size);
        if (msg.free_ptr) {
            msg.free_ptr(NULL, msg.data);
        }
        return -1;
    }

    // 拷贝数据到缓冲区
    memcpy(buffer, msg.data, msg.length);
    *recv_len = msg.length;

    // 释放消息数据
    if (msg.free_ptr) {
        msg.free_ptr(NULL, msg.data);
    }

    return 0;
}

/**
 * @brief 构造消息结构体
 * @param data 数据指针
 * @param length 数据长度
 * @return 构造好的消息结构体
 */
static QueueMessage create_queue_message(void *data, size_t length) {
    QueueMessage msg = {
        .data = data,
        .length = length,
        .free_ptr = free_message_memory
    };
    return msg;
}

/**
 * @brief 清理消息内存池
 */
void deinit_message_pool(void) {
    if (g_message_pool) {
        destroy_memory_pool(g_message_pool);
        g_message_pool = NULL;
    }
}

/**
 * @brief 为消息分配并复制数据
 * @param src 源数据
 * @param length 数据长度
 * @return 成功返回数据指针，失败返回NULL
 */
static void* create_message_data(const void *src, size_t length) {
    if (!src || length <= 0 || length > MESSAGE_BLOCK_SIZE) {
        fprintf(stderr, "Invalid parameters or data too large for message pool\n");
        return NULL;
    }

    // 确保内存池已初始化
    if (!g_message_pool && init_message_pool() != 0) {
        return NULL;
    }

    // 从内存池分配内存
    void *data_copy = allocate_memory(g_message_pool);
    if (!data_copy) {
        fprintf(stderr, "Failed to allocate memory from pool\n");
        return NULL;
    }
    
    // 复制数据
    memcpy(data_copy, src, length);
    return data_copy;
}



/**
 * @brief 发送消息到队列
 * @param mq 消息队列描述符
 * @param msg 消息结构体指针
 * @return 成功返回0，失败返回-1
 */
static int dispatch_queue_message(mqd_t mq, const QueueMessage *msg) {
    if (!msg || mq == (mqd_t)-1) {
        return -1;
    }

    if (mq_send(mq, (const char *)msg, sizeof(QueueMessage), 0) == -1) {
        fprintf(stderr, "Failed to send message: %s\n", strerror(errno));
        return -1;
    }

    return 0;
}


/**
 * @brief 向指定的消息队列发送数据
 * @param mq 消息队列描述符
 * @param data 要发送的数据
 * @param length 数据长度
 * @param func_code 功能码
 * @return 成功返回0，失败返回-1
 */
int dispatch_modbus_message(mqd_t mq, const void *data, size_t length, int func_code) {
    if (!data || length <= 0 || mq == (mqd_t)-1) {
        fprintf(stderr, "Invalid parameters for sending message\n");
        return -1;
    }

    // 创建消息数据
    void *msg_data = create_message_data(data, length);
    if (!msg_data) {
        return -1;
    }

    // 构造消息
    QueueMessage msg = create_queue_message(msg_data, length);

    // 发送消息
    if (dispatch_queue_message(mq, &msg) != 0) {
        if (msg.free_ptr) {
            msg.free_ptr(NULL, msg.data);
        }
        return -1;
    }

    return 0;
}


/**
 * @brief 根据功能码查找对应的接收队列
 * @param func_code 功能码
 * @return 成功返回队列描述符，失败返回-1
 */
static mqd_t find_recv_queue(int func_code) {
    for (int i = 0; i < command_count; i++) {
        if (command_queues[i].func_code == func_code) {
            return command_queues[i].recv_que;
        }
    }
    return (mqd_t)-1;
}

/**
 * @brief 根据功能码向指定模块发送命令
 * @param func_code 功能码
 * @param data 要发送的数据
 * @param length 数据长度
 * @return 成功返回0，失败返回-1
 */
int dispatch_modbus_command(int func_code, const void *data, size_t length) {
    mqd_t recv_que = (mqd_t)-1;
    
    // 查找对应的接收队列
    for (int i = 0; i < command_count; i++) {
        if (command_queues[i].func_code == func_code) {
            recv_que = command_queues[i].recv_que;
            break;
        }
    }
    
    if (recv_que == (mqd_t)-1) {
        fprintf(stderr, "No queue found for function code: %d\n", func_code);
        return -1;
    }

    return send_modbus_message(recv_que, data, length, func_code);
}

// 在register_message_queues函数中添加互斥保护
void register_message_queues(mqd_t* recv_que, mqd_t* send_que, int func_code, ModuleInitFuncPtr init_func) {
    pthread_mutex_lock(&dispatcher_mutex);
    
    *send_que = modbus_send_que;

    if (command_count < MAX_COMMANDS) {
        command_queues[command_count].func_code = func_code;
        command_queues[command_count].recv_que = *recv_que;
        command_count++;
        
        pthread_mutex_unlock(&dispatcher_mutex);
        
        // 调用模块初始化函数
        if (init_func) {
            init_func();
        }
    } else {
        pthread_mutex_unlock(&dispatcher_mutex);
        fprintf(stderr, "Command queue array is full!\n");
    }
}
/**
 * @brief 检查队列状态
 * @param mq 队列描述符
 * @return 队列当前消息数量，失败返回-1
 */
int get_queue_status(mqd_t mq) {
    struct mq_attr attr;
    
    if (mq_getattr(mq, &attr) == -1) {
        fprintf(stderr, "Failed to get queue attributes: %s\n", strerror(errno));
        return -1;
    }
    
    return attr.mq_curmsgs;
}

/**
 * @brief 注销功能码对应的队列
 * @param func_code 功能码
 * @return 成功返回0，失败返回-1
 */
int unregister_message_queue(int func_code) {
    pthread_mutex_lock(&dispatcher_mutex);
    
    for (int i = 0; i < command_count; i++) {
        if (command_queues[i].func_code == func_code) {
            // 关闭队列
            if (command_queues[i].recv_que != (mqd_t)-1) {
                mq_close(command_queues[i].recv_que);
            }
            
            // 移动后面的元素
            if (i < command_count - 1) {
                memmove(&command_queues[i], &command_queues[i + 1], 
                        (command_count - i - 1) * sizeof(CommandQueue));
            }
            
            command_count--;
            pthread_mutex_unlock(&dispatcher_mutex);
            return 0;
        }
    }
    
    pthread_mutex_unlock(&dispatcher_mutex);
    return -1;
}

/**
 * @brief 初始化Modbus调度器
 * @return 成功返回0，失败返回-1
 */
int init_modbus_dispatcher(void) {
    // 创建发送队列
    struct mq_attr attr = {
        .mq_flags = 0,
        .mq_maxmsg = MAX_QUEUE_SIZE,
        .mq_msgsize = sizeof(QueueMessage),
        .mq_curmsgs = 0
    };

    modbus_send_que = mq_open("/modbus_send_queue", O_CREAT | O_RDWR, 0666, &attr);
    if (modbus_send_que == (mqd_t)-1) {
        fprintf(stderr, "Failed to create send queue: %s\n", strerror(errno));
        return -1;
    }

    // 初始化命令队列数组
    memset(command_queues, 0, sizeof(command_queues));
    command_count = 0;

    return 0;
}

/**
 * @brief 清理Modbus调度器
 */
void deinit_modbus_dispatcher(void) {
    // 关闭所有队列
    for (int i = 0; i < command_count; i++) {
        if (command_queues[i].recv_que != (mqd_t)-1) {
            mq_close(command_queues[i].recv_que);
        }
    }

    // 关闭并删除发送队列
    if (modbus_send_que != (mqd_t)-1) {
        mq_close(modbus_send_que);
        mq_unlink("/modbus_send_queue");
    }
}


void modbus_app_module_init(void) {
    init_modbus_dispatcher();
    register_ota_message_queue();
}

void modbus_app_module_deinit(void) {
    deinit_modbus_dispatcher();
}