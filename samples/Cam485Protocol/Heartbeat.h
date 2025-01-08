#ifndef HEARTBEAT_H_
#define HEARTBEAT_H_

#include <stdio.h>
#include <stdint.h>
#include "CommonInterface.h"

//主地址与从地址
#define CMD_HEARTBEAT 0x01
#define SUBCMD_HEARTBEAT 0x01
#define HEART_FRAME_SIZE 11
#define PAYLOAD_SIZE 1 

//心跳帧
typedef struct {
    uint8_t header[2];      // 帧头
    uint8_t command;        // 主命令
    uint8_t sub_command;    // 子命令
    uint8_t payload[PAYLOAD_SIZE]; // 数据
    uint8_t reserved[4];
    uint16_t checksum;      // 校验和
} HeartFrame_t;

int SID01_HeartBeat(uint8_t* msg_buf, uint16_t msg_dlc);
int SID01_CheckHeartBeat(const uint8_t* msg_buf, uint16_t msg_dlc);


#endif // HEARTBEAT_H_