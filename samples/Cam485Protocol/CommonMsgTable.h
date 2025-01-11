#ifndef CommonMsgTable_h
#define CommonMsgTable_h

#include "stdint.h"
#include "stdbool.h"
#include "stdio.h"

#define SID_NUM       16     // 当前共支持 16 个服务

#define SID_01        (0x01) /* 心跳 */
#define SID_06        (0x06) /* 主动抓图 */

typedef struct 
{
    uint8_t sid;									// 数据帧id
	void (* msg_process_callback)  (const uint8_t *, uint16_t);	// 消息处理函数
	// bool_t (* check_len)  (const uint8_t *, uint16_t);	// 检查数据长度是否合法
}MsgService;

extern const MsgService MsgServiceList[SID_NUM];
#endif