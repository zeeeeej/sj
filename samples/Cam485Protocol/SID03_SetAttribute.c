#include "cm_common.h"
#include "stdbool.h"
#include "SID03_SetAttribute.h"
#include "Cam485ProtocolCommon.h"
#include "ImageInfoList.h"
#include "elog.h"
#include "AttributeTable.h"
#define LOG_TAG "[SID03_SetAttribute]"




// 定义错误码
#define ERR_SUCCESS 0
#define ERR_INVALID_PARAMS -1
#define ERR_SYSTEM_ERROR -2



// 验证消息格式
static int validate_message(const uint8_t* msg_buf, uint32_t msg_dlc) {
    if (msg_buf == NULL || msg_dlc < 10) {
        log_e("Invalid message parameters");
        return ERR_INVALID_PARAMS;
    }

    if (msg_buf[0] != 0xAA || msg_buf[1] != 0x5A) {
        log_e("Invalid frame header");
        return ERR_INVALID_PARAMS;
    }

    uint32_t len = (msg_buf[4]) | (msg_buf[5] << 8) | (msg_buf[6] << 16) | (msg_buf[7] << 24);
    if (len != msg_dlc - 10) {
        log_e("Data length mismatch");
        return ERR_INVALID_PARAMS;
    }

    return ERR_SUCCESS;
}

// 主处理函数
int SID03_SetAttribute(uint8_t *msg_buf, uint32_t msg_dlc) {
    log_i("SID03_SetAttribute");
    int result = validate_message(msg_buf, msg_dlc);
    if (result != ERR_SUCCESS) {
        return result;
    }

    uint8_t id = msg_buf[8];
    const uint8_t* value = &msg_buf[9];
    /*total length - 4 header -4 data length - 2 crc - 1 id*/
    uint32_t value_len = msg_dlc - 10 -1;
    log_i("id: %d", id);
    log_i("value_len: %d", value_len);
    
    const AttributeEntry* entry = find_attribute_handler(id);
    if (!entry) {
        log_e("Unknown attribute ID: 0x%02X", id);
        return ERR_INVALID_PARAMS;
    }
    
    if (value_len < entry->expected_value_len) {
        log_e("Value length too short for attribute 0x%02X", id);
        return ERR_INVALID_PARAMS;
    }
    log_i("process attribute id: %d", id);
    result = entry->set_handler(value, value_len);
    return ERR_SUCCESS;
}






