//
// Created by DELL on 24-9-20.
//

#ifndef SAMPLES_WIND_PROCESS_MANAGER_H
#define SAMPLES_WIND_PROCESS_MANAGER_H

enum {
    PROCESS_STATUS_IDLE = 0, // wait some command
    PROCESS_STATUS_STARTING, // system starting
    PROCESS_STATUS_MPU_STARING, // starting mpu
    PROCESS_STATUS_DOOR_OPEN, // door open
};

enum {
    CONNECT_STATUS_IDLE = 0,
    CONNECT_STATUS_CONFIG,
    CONNECT_STATUS_IMAGE_TRANSFER,
};

typedef struct
{
    int status;
    int connect_status;
} WindProcessManager;

int wind_process_manager_status_changed(int st);
int wind_process_manager_connect_status_changed(int st);

// 获取当前状态
int wind_process_manager_get_status(void);
// 获取当前连接状态
int wind_process_manager_get_connect_status(void);

#endif //SAMPLES_WIND_PROCESS_MANAGER_H
