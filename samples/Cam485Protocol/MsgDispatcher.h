#ifndef MsgDispatcher
#define MsgDispatcher

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
uint16_t crc16(uint8_t *q, int len);
#endif