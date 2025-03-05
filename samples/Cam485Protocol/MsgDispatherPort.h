#ifndef MSG_DISPATHER_PORT_H__
#define MSG_DISPATHER_PORT_H__


#include "stdint.h"
#include "MsgDispatcher.h"
// 定义控制码
#define UART_CTRL_SET_BAUDRATE 0
#define UART_CTRL_GET_BAUDRATE    1  // 新增获取波特率控制码
DataTransInterface* get_uart_interface(void);
#endif