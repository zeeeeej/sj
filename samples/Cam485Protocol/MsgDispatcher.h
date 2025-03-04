#ifndef MsgDispatcher
#define MsgDispatcher

#include "ParseIni.h"
#include <stdint.h>


#define SEND_DEBUG_ENABLE 1

/*默认从机地址*/
#define SLAVE_ADDR_DEFAULT          (1)
/*广播地址*/
#define BROADCAST_ADDR              (0xFF)

#define SEND_MSG_BLOCK      1

#define MSG_DISPATHER_RECV_DEBUG_EN  0
#define MSG_DISPATHER_SEND_DEBUG_EN  0

/*接收单次数据最大长度*/
#define MAX_RECV_MSG_LEN (1024*7)
#define READ_TIME_OUT_MS  300


typedef struct DataTransInterface {
    int (*send_data)(uint8_t *data, uint32_t len);
    int (*recv_data)(uint8_t *data, uint32_t len,int time_out_ms);
    int (*init)();
    int (*control)(int control_code, void *user_data,uint32_t len);
} DataTransInterface;

int Cam485ProtocolInit();
uint8_t send_msg_resp(uint8_t *msg, uint32_t len);
uint8_t send_msg_image(uint8_t *msg, uint32_t len);
uint16_t crc16(uint8_t *buffer, uint32_t buffer_length);
uint16_t image_crc16(uint16_t crc, uint8_t *buffer, uint32_t buffer_length);
int set_slave_address(uint8_t slave_address_param);
int get_slave_address(uint8_t *slave_address_param);
int Get_CurRecvSlaveAddress(uint8_t *slave_address);
#endif