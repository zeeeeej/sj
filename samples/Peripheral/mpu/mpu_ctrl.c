#include "yq_mpu.h"
#include "elog.h"
#include <pthread.h>
#include <string.h>

#define LOG_TAG "[GYRO_CTRL]"

// 错误码定义
#define GYRO_OK             0
#define GYRO_ERR_NULLPTR   -1
#define GYRO_ERR_NOT_INIT  -2
#define GYRO_ERR_ALREADY   -3
#define GYRO_ERR_OPERATION -4

// 陀螺仪控制结构体
typedef struct {
    const YQMpuImpl* impl;     // 陀螺仪实现接口
    YQMpuContext     ctx;      // 陀螺仪上下文
    int             initialized;// 初始化标志
    pthread_mutex_t  mutex;    // 互斥锁
} GyroCtrl;

// 全局陀螺仪控制实例
static GyroCtrl s_gyro_ctrl = {
    .impl = &yq_mpu_impl_lsm6ds3trc,
    .initialized = 0,
};

// 内部函数：检查陀螺仪状态
static inline int check_gyro_status(void)
{
    if (s_gyro_ctrl.impl == NULL) {
        log_e("Gyroscope implementation is NULL");
        return GYRO_ERR_NULLPTR;
    }
    
    if (!s_gyro_ctrl.initialized) {
        log_e("Gyroscope not initialized");
        return GYRO_ERR_NOT_INIT;
    }
    
    return GYRO_OK;
}

// 陀螺仪初始化
int gyroscope_open(void)
{
    int ret;

    // 检查是否已初始化
    if (s_gyro_ctrl.initialized) {
        log_w("Gyroscope already initialized");
        return GYRO_OK;
    }
    
    // 初始化互斥锁
    if (pthread_mutex_init(&s_gyro_ctrl.mutex, NULL) != 0) {
        log_e("Failed to initialize mutex");
        return GYRO_ERR_OPERATION;
    }
    
    // 调用实现的初始化函数
    ret = s_gyro_ctrl.impl->open(&s_gyro_ctrl.ctx, s_gyro_ctrl.impl->device);
    if (ret != GYRO_OK) {
        pthread_mutex_destroy(&s_gyro_ctrl.mutex);
        log_e("Failed to initialize gyroscope");
        return ret;
    }
    
    s_gyro_ctrl.initialized = 1;
    log_i("Gyroscope initialized successfully");

    return GYRO_OK;
}

// 陀螺仪反初始化
int gyroscope_close(void)
{
    int ret;
    
    ret = check_gyro_status();
    if (ret != GYRO_OK) {
        return ret;
    }
    
    pthread_mutex_lock(&s_gyro_ctrl.mutex);
    // 调用实现的清理函数
    ret = s_gyro_ctrl.impl->close(&s_gyro_ctrl.ctx);
    if (ret != GYRO_OK) {
        log_e("Failed to close gyroscope");
    }
    
    s_gyro_ctrl.initialized = 0;
    pthread_mutex_unlock(&s_gyro_ctrl.mutex);
    pthread_mutex_destroy(&s_gyro_ctrl.mutex);
    
    log_i("Gyroscope closed");
    return ret;
}

// 检查陀螺仪是否就绪
int gyroscope_ready(void)
{
    int ret;
    
    ret = check_gyro_status();
    if (ret != GYRO_OK) {
        return ret;
    }
    
    pthread_mutex_lock(&s_gyro_ctrl.mutex);
    ret = s_gyro_ctrl.impl->ready(&s_gyro_ctrl.ctx);
    pthread_mutex_unlock(&s_gyro_ctrl.mutex);
    
    return ret;
}

// 读取陀螺仪YPR数据
int gyroscope_read_ypr(float ypr[3])
{
    int ret;
    
    ret = check_gyro_status();
    if (ret != GYRO_OK) {
        return ret;
    }

    pthread_mutex_lock(&s_gyro_ctrl.mutex);
    if (s_gyro_ctrl.impl->read_ypr(&s_gyro_ctrl.ctx, ypr) <= 0) {
        pthread_mutex_unlock(&s_gyro_ctrl.mutex);
        return -1;
    }
    pthread_mutex_unlock(&s_gyro_ctrl.mutex);

    return 0;
}

// 读取陀螺仪和加速度数据
int gyroscope_read_gyro(float gyro[3], float accel[3])
{
    int ret;
    
    ret = check_gyro_status();
    if (ret != GYRO_OK) {
        return ret;
    }

    pthread_mutex_lock(&s_gyro_ctrl.mutex);
    if(s_gyro_ctrl.impl->read_gyro(&s_gyro_ctrl.ctx, gyro, accel) <= 0) {
        pthread_mutex_unlock(&s_gyro_ctrl.mutex);
        return -1;
    }
    pthread_mutex_unlock(&s_gyro_ctrl.mutex);
    
    return 0;
}