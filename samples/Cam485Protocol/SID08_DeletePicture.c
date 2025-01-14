#include "SID08_DeletePicture.h"


int checkDeletePhoto(const uint8_t *msg_buf, uint16_t msg_len)
{
    if (msg_buf == NULL || msg_len < 8) 
    {
        LOGD("DeletePhoto Invalid message buffer\n");
        return -1;
    }
    if (msg_buf[2] < 0x01 || msg_buf[2] > 0x10) {
        LOGD("Invalid slave address\n");
        return -1;
    }
    const uint8_t *data = &msg_buf[8];

    // 数据段长度直接从帧的长度推算：去掉帧头和校验的长度
    uint16_t data_length = msg_len - 8 - 2; // 8字节头部 + 1字节校验

    printf("Data length: %d\n", data_length);
    printf("Data segment: ");
    for (uint16_t i = 0; i < data_length; i++) {
        printf("0x%02X ", data[i]);
    }
    printf("\n");
    return data_length;
}

int HandleDeletePhoto(uint8_t *msg_buf, uint16_t msg_len)
{
    int datalen = checkDeletePhoto(msg_buf, msg_len);
    uint8_t *data = &msg_buf[8];
    LOGD("Received data:");
    for (int i = 0; i < datalen; i++) {
        printf("%02X ", data[i]);
    }
    printf("\n");

    // 执行删除操作
    int ret = DeletePicture(data[0]); // 假设数据的第1字节是照片ID
    uint8_t response[1024] = {0};
    uint16_t index = 0;

    // 构造响应帧
    response[index++] = 0xAA;  // 帧头
    response[index++] = 0x5A;
    response[index++] = 0x01;
    response[index++] = 0x08;  // 主命令
    response[index++] = 0x01;  // 子命令
    response[index++] = 0x00;  // 子命令
    response[index++] = 0x00;  // 子命令
    response[index++] = 0x00;  // 子命令
    response[index++] = (ret == 0) ? 0x00 : 0x01; // 返回值：0x00成功，0x01失败

    // 发送响应帧
    send_msg_resp(response, index);

    if (ret != 0) {
        LOGD("Failed to delete photo, ret[%d]\n", ret);
        return -1;
    }
    return 0;
}

int DeletePicture(const uint8_t pic_id)
{
    int ret = 0;

    if (pic_id == 0xFF) 
    {
        // 删除 1~10 号照片
        for (int i = 1; i <= 10; i++) 
        {
            char photo_file[256];
            snprintf(photo_file, sizeof(photo_file), "%s%d.jpg", TAKE_PHOTO_TMP_FILE, i);
            if (access(photo_file, F_OK) == 0) 
            { 
                if (unlink(photo_file) != 0) 
                {
                    fprintf(stderr, "Failed to delete photo %s: %s\n", photo_file, strerror(errno));
                    ret = -1;
                } 
                else 
                {
                    printf("Deleted photo: %s\n", photo_file);
                }
            }
        }
        if(activeTriggerList == NULL)
        {
            printf("activeTriggerList is NULL\n");
        }
        else
        {
            printList(activeTriggerList);
            deleteAllNodes(activeTriggerList); 
            printf("After delete\n");
            printList(activeTriggerList);
        }
        
    } 
    else 
    {
        // 删除单张照片
        char photo_file[256];
        snprintf(photo_file, sizeof(photo_file), "%s%d.jpg", TAKE_PHOTO_TMP_FILE, pic_id);
        if (access(photo_file, F_OK) == 0) 
        { // 检查文件是否存在
            if (unlink(photo_file) != 0) 
            {
                fprintf(stderr, "Failed to delete photo %s: %s\n", photo_file, strerror(errno));
                ret = -1;
            } 
            else 
            {
                printf("Deleted photo: %s\n", photo_file);
            }

        }
        else 
        {
            fprintf(stderr, "Photo file %s not found.\n", photo_file);
            ret = -1;
        }
        if(activeTriggerList == NULL)
        {
            printf("activeTriggerList is NULL\n");
        }
        else
        {
            printList(activeTriggerList);

            //删除链表中的节点
            deleteNodeById(activeTriggerList, pic_id); 

            printf("After delete\n");
            printList(activeTriggerList);
        }

        
    }

    return ret;
}

