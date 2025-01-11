/*主动拍照*/
#include "Heartbeat.h"
#include "cm_common.h"

#include "stdbool.h"
#include "SID06_ActiveScreenshot.h"
#include "ProtocolPort.h"
/*主动抓图的id值*/
/*开关门触发的id值*/
/*这两个是否冲突*
文件存放的位置是应该不一样*/
static uint8_t image_id = 0;

static uint8_t get_image_id()
{
    image_id++;
    if (image_id > 100)
    {
        image_id = 0;
    }
}

bool SID06_ActiveScreenshot_check_len(const uint8_t *msg_buf, uint16_t msg_dlc)
{
    bool ret = -1;

    (void)msg_buf;
    if (msg_dlc != 2)
        ret = 0;

    return ret;
}

static int SID06_BuildMsgHeader(uint8_t *msg_buf, uint16_t msg_dlc)
{
    int index = -1;
    msg_buf[index++] = 0x5A;
    msg_buf[index++] = 0xAA;
    msg_buf[index++] = SLAVE_ADDR;
    msg_buf[index++] = 0x06;

    /*填充长度字段*/
    msg_buf[index++] = (uint8_t)(msg_dlc & 0xFF);         // Low byte
    msg_buf[index++] = (uint8_t)((msg_dlc >> 8) & 0xFF);  // 2nd byte
    msg_buf[index++] = (uint8_t)((msg_dlc >> 16) & 0xFF); // 3rd byte
    msg_buf[index++] = (uint8_t)((msg_dlc >> 24) & 0xFF); // High byte
}

static int SID06_BuildMsgResp(uint8_t *msg_buf, uint16_t msg_dlc)
{

}

int SID06_ActiveScreenshot(uint8_t *msg_buf, uint16_t msg_dlc)
{
    int ret = 0;
    uint8_t result = 0;
    uint32_t id;
    ret = SID06_ActiveScreenshot_check_len(msg_buf, msg_dlc);
    if (ret != 0)
    {
        return -1;
    }
    char filename[50];    
    id = get_image_id();                                                      // Buffer to hold the full filename
    sprintf(filename, "%simage%d.jpg", ACTIVE_TRIGGER_PHOTO_FILE_DIR, id); // Create full filename with directory
    result = request_take_photo(filename); // Pass the full filename to the function
    char resp_buf[SID06_MSG_RESP_TOTAL_LEN];
    SID06_BuildMsgResp(resp_buf,SID06_MSG_RESP_DATA_LEN);
    msg_buf[8] = result;
    msg_buf[9] = id;
    send_msg_resp(resp_buf,SID06_MSG_RESP_DATA_LEN);
}
