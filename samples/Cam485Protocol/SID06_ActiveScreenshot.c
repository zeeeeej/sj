/*主动拍照*/
#include "cm_common.h"

#include "stdbool.h"
#include "SID06_ActiveScreenshot.h"
#include "Cam485ProtocolCommon.h"
#include "ImageInfoList.h"
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

int SID06_ActiveScreenshotCheck(const uint8_t *msg_buf, uint32_t msg_dlc)
{
    int ret = 0;

    (void)msg_buf;
    if (msg_dlc < SID06_MSG_REQ_TOTAL_LEN)  
        ret = -1;

    return ret;
}

static int SID06_BuildMsgHeader(uint8_t *msg_buf, uint32_t msg_dlc)
{
    int index = 0;
    msg_buf[index++] = 0xAA;
    msg_buf[index++] = 0x5A;
    msg_buf[index++] = SLAVE_ADDR;
    msg_buf[index++] = 0x06;

    /*填充长度字段*/
    msg_buf[index++] = (uint8_t)(msg_dlc & 0xFF);         // Low byte
    msg_buf[index++] = (uint8_t)((msg_dlc >> 8) & 0xFF);  // 2nd byte
    msg_buf[index++] = (uint8_t)((msg_dlc >> 16) & 0xFF); // 3rd byte
    msg_buf[index++] = (uint8_t)((msg_dlc >> 24) & 0xFF); // High byte
}           

int SID06_ActiveScreenshot(uint8_t *msg_buf, uint32_t msg_dlc)
{
    LOGD("SID06_ActiveScreenshot");
    uint8_t resp_buf[SID06_MSG_RESP_TOTAL_LEN];
    memset(resp_buf, 0, SID06_MSG_RESP_TOTAL_LEN);
    int ret = 0;
    uint8_t result = 0;
    uint8_t id;
    ret = SID06_ActiveScreenshotCheck(msg_buf, msg_dlc);
    if (ret != 0)
    {
        return -1;
    }
    char filename[50];
    id = get_image_seq();                                                   // Buffer to hold the full filename
    sprintf(filename, "%simage_%d.jpg", ACTIVE_TRIGGER_PHOTO_FILE_DIR, id); // Create full filename with directory
    result = request_take_photo(filename);                                  // Pass the full filename to the function
    printf("result = %d", result);
    printf("id = %d", id);
    generate_image_info(filename);  
    
    SID06_BuildMsgHeader(resp_buf, SID06_MSG_RESP_DATA_LEN);
    resp_buf[8] = result;
    resp_buf[9] = id;
    printf("After header: msg_buf[8] = 0x%02X, msg_buf[9] = 0x%02X\n", resp_buf[8], resp_buf[9]);
    send_msg_resp(resp_buf, SID06_MSG_RESP_TOTAL_LEN);
}
