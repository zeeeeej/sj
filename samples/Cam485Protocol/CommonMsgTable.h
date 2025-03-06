#ifndef CommonMsgTable_h
#define CommonMsgTable_h

#include "stdint.h"
#include "stdbool.h"
#include "stdio.h"

#define SID_NUM       16     // 当前共支持 16 个服务

#define SID_01        (0x01) /* 心跳 */
#define SID_02        (0x02) /* 查询属性 */
#define SID_03        (0x03) /* 设置属性 */
#define SID_04        (0x04) /* 恢复出厂设置 */
#define SID_05        (0x05) /* 重启摄像头 */
#define SID_06        (0x06) /* 主动抓图 */
#define SID_08		  (0x08) /* 删除图片 */
#define SID_07		  (0x07) /*查询存储的图片信息*/	
#define SID_09		  (0x09) /*获取图片*/
#define SID_0A		  (0x0A) /*图片获取完成*/
#define SID_0B		  (0x0B) /*固件升级通知*/
#define SID_0C		  (0x0C) /*发送固件升级包*/
#define SID_0D		  (0x0D) /*门开关信号*/
#define SID_FE		  (0xFE) /*core文件传输*/
#define SID_FF		  (0xFF) /*测试通道*/	
#define SID_F1		  (0xF1) /*PWM控制*/
typedef struct 
{
    uint8_t sid;									// 数据帧id
	int (* msg_process_callback)  (uint8_t *msg_buf , uint32_t msg_dlc);	// 消息处理函数
	// bool_t (* check_len)  (const uint8_t *, uint16_t);	// 检查数据长度是否合法
}MsgService;

extern const MsgService MsgServiceList[SID_NUM];
#endif