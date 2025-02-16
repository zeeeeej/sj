/*查询属性*/
#include "cm_common.h"

#include "stdbool.h"
#include "SID02_QueryAttribute.h"
#include "Cam485ProtocolCommon.h"
#include "AttributeTable.h"
#include "elog.h"
#define LOG_TAG "[SID02_QueryAttribute]"

// 定义错误码
#define ERR_SUCCESS 0
#define ERR_INVALID_PARAMS -1
#define ERR_SYSTEM_ERROR -2

int SID02_QueryAttribute(uint8_t *msg_buf, uint32_t msg_dlc)
{
    log_i("SID02_QueryAttribute");
    if (msg_buf == NULL || msg_dlc != 11) 
    {
        log_e("error:msg_len = %d",msg_dlc);
        return ERR_INVALID_PARAMS;
    }

    uint8_t command    = msg_buf[2];
    uint8_t subcommand = msg_buf[3];
    uint32_t len = (msg_buf[4]) | (msg_buf[5] << 8) | (msg_buf[6] << 16) | (msg_buf[7] << 24);
    log_i("len:%d\n",len);
    if (len != msg_dlc - 10)  //// 10字节 = 帧头(2) + 命令(2) + 长度(4) + 校验码(2)
    {
        log_e("Error: Data length mismatch\n");
        return ERR_INVALID_PARAMS;
    }
    uint8_t id = msg_buf[8];           // 属性 ID
    
    log_i("Command: 0x%02X, Subcommand: 0x%02X", command, subcommand);
    log_i("Attribute ID: 0x%02X", id);

    // 查找属性处理函数
    const AttributeEntry* entry = find_attribute_handler(id);
    if (!entry) {
        log_e("Unknown attribute ID: 0x%02X", id);
        return ERR_INVALID_PARAMS;
    }

    // 准备缓冲区存储查询结果
    uint8_t value_buffer[32];  // 根据实际需要调整大小
    uint32_t value_len = entry->expected_value_len;
    
    // 调用属性的查询处理函数
    return entry->get_handler(value_buffer, value_len);
}
