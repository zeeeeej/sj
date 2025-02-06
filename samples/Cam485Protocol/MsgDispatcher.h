#ifndef MsgDispatcher
#define MsgDispatcher

#include "ParseIni.h"
#include <stdint.h>


#define SLAVE_ADDR 0x01

#define MSG_DISPATHER_RECV_DEBUG_EN  1
#define MSG_DISPATHER_SEND_DEBUG_EN  0
typedef struct DataTransInterface {
    uint8_t (*send_data)(uint8_t *data, uint32_t len);
    uint8_t (*recv_data)(uint8_t *data, uint32_t len);
    uint8_t (*init)();
} DataTransInterface;

void Cam485ProtocolInit();
uint8_t send_msg_resp(uint8_t *msg, uint32_t len);
uint8_t send_msg_image(uint8_t *msg, uint32_t len);
uint16_t crc16(uint8_t *buffer, uint16_t buffer_length);
uint16_t image_crc16(uint16_t crc, uint8_t *buffer, uint16_t buffer_length);
#endif