#ifndef MsgDispatcher
#define MsgDispatcher

#define SLAVE_ADDR 0x01

#define MSG_DISPATHER_DEBUG_EN  1

typedef struct DataTransInterface {
    uint8_t (*send_data)(uint8_t *data, uint8_t len);
    uint8_t (*recv_data)(uint8_t *data, uint8_t len);
    uint8_t (*init)();
} DataTransInterface;

void Cam485ProtocolInit();
uint8_t send_msg_resp(uint8_t *msg, uint8_t len);
#endif