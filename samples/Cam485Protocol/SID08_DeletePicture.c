#include "SID08_DeletePicture.h"
#include "Cam485ProtocolCommon.h"
#include "elog.h"



#define LOG_TAG   "[SID08->delete photo]"
static int checkDeletePhoto(const uint8_t *msg_buf, uint32_t msg_len)
{
    if (msg_buf == NULL || msg_len < 8) 
    {
        LOGD("DeletePhoto Invalid message buffer\n");
        return -1;
    }
    if (msg_buf[2] < 0x01 || msg_buf[2] > 0x10) {
        LOGD("Invalid slave address");
        return -1;
    }
    const uint8_t *data = &msg_buf[8];

    // 数据段长度直接从帧的长度推算：去掉帧头和校验的长度
    uint32_t data_length = msg_len - 8 - 2; // 8字节头部 + 1字节校验

    printf("Data length: %d\n", data_length);
    printf("Data segment: ");
    for (uint32_t i = 0; i < data_length; i++) {
        printf("0x%02X ", data[i]);
    }
    printf("\n");
    return data_length;
}



static int DeletePicture(const uint8_t pic_id)
{
    int ret = 0;

    /*如果需要删除所有图片*/
    if (pic_id == 0xFF) 
    {
        /*删除所有图片*/
        /*删除两个目录，然后重新创建目录*/
        log_i("delete all pic");
        remove_dir(ACTIVE_TRIGGER_PHOTO_FILE_DIR);
        remove_dir(GYRO_TRIGGER_PHOTO_FILE_DIR);
        check_and_create_dir(ACTIVE_TRIGGER_PHOTO_FILE_DIR);
        check_and_create_dir(GYRO_TRIGGER_PHOTO_FILE_DIR);

        /*清空所有链表节点*/
        cleanAllLinkList();
    } 
    else 
    {
        // 删除单张照片
        char file_path[255];
        int ret =  0;
        ret = find_file_path_by_id(pic_id,file_path);
        if(ret != 0)
        {
            log_w("not found image");
        }
        else
        {
            log_d("found image path : %s",file_path);
        }

        if (access(file_path, F_OK) == 0) 
        { // 检查文件是否存在
            if (unlink(file_path) != 0) 
            {
                log_e("Failed to delete photo %s: %s", file_path, strerror(errno));
                ret = -1;
            } 
            else 
            {
                log_i("Deleted photo: %s", file_path);
            }

        }
        else 
        {
            log_e("file path : %s access error",file_path);
            ret = -1;
        }
        deleteListNodeById(pic_id);
    }
    return ret;
}

int SID08_HandleDeletePhoto(uint8_t *msg_buf, uint32_t msg_len)
{
    log_i("handle delete photo");
    int datalen = checkDeletePhoto(msg_buf, msg_len);
    uint8_t *data = &msg_buf[8];
    printf("\n");

    // 执行删除操作
    int ret = DeletePicture(data[0]); // 假设数据的第1字节是照片ID
    uint8_t response[1024] = {0};
    uint32_t index = 0;
    uint8_t slave_address;
    Get_Protocol_Slave_Address(&slave_address);
    // 构造响应帧
    response[index++] = 0xAA;  // 帧头
    response[index++] = 0x5A;
    response[index++] = slave_address;
    response[index++] = 0x08;  // 主命令
    response[index++] = 0x01;  // 子命令
    response[index++] = 0x00;  // 子命令
    response[index++] = 0x00;  // 子命令
    response[index++] = 0x00;  // 子命令
    response[index++] = (ret == 0) ? 0x00 : 0x01; // 返回值：0x00成功，0x01失败

    // 发送响应帧
    send_msg_resp(response, index + 2);

    if (ret != 0) {
        LOGD("Failed to delete photo, ret[%d]", ret);
        return -1;
    }
    log_i("DELETE PHOTO END");
    return 0;
}