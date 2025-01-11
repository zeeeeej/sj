#include "SendPhoto.h"

int ParseGetPictureRequest(const uint8_t *msg_buf, uint16_t msg_len,GetPictureRequest_t *req)
{
    if (msg_buf == NULL || req == NULL || msg_len < 12) 
    {
        return -1;
    }
    if (msg_buf[0] != 0xAA || msg_buf[1] != 0x5A || msg_buf[2] != 0x09) 
    {
        return -1;
    }
    req->header[0] = msg_buf[0];
    req->header[1] = msg_buf[1];
    req->command = msg_buf[2];
    req->sub_command = msg_buf[3];
    req->pic_id = msg_buf[4];
    req->offset = (msg_buf[5] << 24) | (msg_buf[6] << 16) | (msg_buf[7] << 8) | msg_buf[8];
    req->read_len = (msg_buf[9] << 8) | msg_buf[10];
    req->checksum = (msg_buf[11] << 8) | msg_buf[12];
    return 0;
}


int PackGetPictureResponse(uint8_t *msg_buf, uint16_t buf_size, uint8_t photo_len,const uint8_t result, const uint8_t *photo_data)
{
    uint8_t index = 0;
    msg_buf[index++] = 0xAA;
    msg_buf[index++] = 0x5A;
    msg_buf[index++] = 0x09;

    msg_buf[index++] = 1 + photo_len;
    msg_buf[index++] = result;
    if (photo_data != NULL && photo_len > 0) 
    {
        memcpy(&msg_buf[index], photo_data, photo_len);
        index += photo_len;
    }
    send_msg_resp(msg_buf, index);
    return index;
}

int HandlePhotoRequest(uint8_t *msg_buf, uint16_t msg_len)
{
    GetPictureRequest_t req;
    if (ParseGetPictureRequest(msg_buf, msg_len, &req) != 0) 
    {
        LOGD("Failed to parse photo request,ret[-1]\n");
        return PackGetPictureResponse(msg_buf, msg_len, 0, -1, NULL);
    }
    if (req.sub_command != 0x01) 
    {
        LOGD("Invalid sub command,ret[-2]\n");
        return PackGetPictureResponse(msg_buf, msg_len, 0, -2, NULL);
    }
    if (req.pic_id != 0) 
    {
        LOGD("Invalid pic id,ret[-3]\n");
        return PackGetPictureResponse(msg_buf, msg_len, 0, -3, NULL);
    }
    if (req.offset == 0) 
    {
        unlink(TAKE_PHOTO_TMP_FILE);
        if (cm_video_take_photo_save_to_file(TAKE_PHOTO_TMP_FILE) != 0) 
        {
            LOGD("Failed to take photo,ret[-4]\n");
            return PackGetPictureResponse(msg_buf, msg_len, 0, -4, NULL);
        }
    }
    FILE *fp = fopen(TAKE_PHOTO_TMP_FILE, "rb");
    if (!fp) 
    {
        LOGD("Failed to open photo file: %s,ret[-5]\n", strerror(errno));
        return PackGetPictureResponse(msg_buf, msg_len, 0, -5, NULL);
    }
    fseek(fp, 0, SEEK_END);
    uint32_t size = ftell(fp);
    if (req.offset >= size) 
    {
        fclose(fp);
        LOGD("offset >= size,ret[-6]\n");
        return PackGetPictureResponse(msg_buf, msg_len, 0, -6, NULL);
    }
    fseek(fp, req.offset, SEEK_SET);
    uint8_t *data = malloc(req.read_len);
    if (!data) 
    {
        fclose(fp);
        LOGD("Failed to allocate memory,ret[-7]\n");
        return PackGetPictureResponse(msg_buf, msg_len, 0, -7, NULL);
    }
    size_t read_len = fread(data, 1, req.read_len, fp);
    fclose(fp);
    if (read_len != req.read_len) 
    {
        free(data);
        LOGD("Failed to read photo data,ret[-8]\n");
        return PackGetPictureResponse(msg_buf, msg_len, 0, -8, NULL);
    }
    uint8_t *base64_buffer = malloc(req.read_len * 3);
    if (!base64_buffer) 
    {
        free(data);
        LOGD("Failed to allocate memory for base64 buffer,ret[-9]\n");
        return PackGetPictureResponse(msg_buf, msg_len, 0, -9, NULL);
    }
    size_t base64_len = req.read_len * 3;
    if (cm_base64_encode((unsigned char *)base64_buffer, &base64_len, data, req.read_len) != 0) 
    {
        free(data);
        free(base64_buffer);
        LOGD("Failed to encode photo data,ret[-10]\n");
        return PackGetPictureResponse(msg_buf, msg_len, 0, -10, NULL);
    }
    int ret = PackGetPictureResponse(msg_buf, msg_len, base64_len, 0, base64_buffer);
    IS_take_photo = 0x00;
    free(data);
    free(base64_buffer);
    return ret;
}

int HandleIsTakePhotoFinshi(uint8_t *msg_buf, uint16_t msg_len)
{
    if (msg_buf == NULL || msg_len < 12) 
    {
        return -1;
    }
    if (msg_buf[0] != 0xAA || msg_buf[1] != 0x5A || msg_buf[2] != 0x0A || msg_buf[3] != 0x00) 
    {
        return -1;
    }
    msg_buf[0] = 0xAA;
    msg_buf[1] = 0x5A;
    msg_buf[2] = 0xAA;
    msg_buf[3] = 0x01;
    msg_buf[4] = IS_take_photo;
    return 0;
}

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
    }

    return ret;
}

int HnadleGetPictureInfo(uint8_t *msg_buf, uint16_t msg_len)
{
    if (msg_buf == NULL || msg_len < 4) {
        LOGD("GetPictureInfo Invalid message buffer\n");
        return -1;
    }

    if (msg_buf[0] != 0xAA || msg_buf[1] != 0x5A || msg_buf[2] != 0x07 || msg_buf[3] != 0x00) {
        LOGD("GetPictureInfo Invalid message header\n");
        return -1;
    }

    // 构建回复数据帧
    uint8_t response[1024] = {0};  
    uint16_t index = 0;

    response[index++] = 0xAA;  // 帧头
    response[index++] = 0x5A;
    response[index++] = 0x07;  // 主命令
    response[index++] = 0x00;  // 子命令

    // 填充图片数量
    response[index++] = MAX_PIC_NUM;

    for (uint8_t i = 0; i < MAX_PIC_NUM; i++) {
        PicInfo_t *pic = &pic_info[i];

        response[index++] = pic->pic_id;                // 图片ID
        response[index++] = pic->trigger_type;         // 触发方式
        response[index++] = pic->trigger_angle;        // 触发角度

        response[index++] = (pic->capture_time >> 24) & 0xFF;  // 抓取时间
        response[index++] = (pic->capture_time >> 16) & 0xFF;
        response[index++] = (pic->capture_time >> 8) & 0xFF;
        response[index++] = pic->capture_time & 0xFF;

        response[index++] = (pic->image_size >> 24) & 0xFF;  // 图片大小
        response[index++] = (pic->image_size >> 16) & 0xFF;
        response[index++] = (pic->image_size >> 8) & 0xFF;
        response[index++] = pic->image_size & 0xFF;

        memcpy(&response[index], pic->md5, MD5_SIZE);  // MD5
        index += MD5_SIZE;
    }

    send_msg_resp(response, index);

    return index;
}

int HandleTakePhoto(uint8_t *msg_buf, uint16_t msg_len)
{
    if (msg_buf == NULL || msg_len < 4) {
        LOGD("TakePhoto Invalid message buffer\n");
        return -1;
    }
    if(msg_buf[0] != 0xAA || msg_buf[1] != 0x5A || msg_buf[2] != 0x06 || msg_buf[3] != 0x00) {
        LOGD("TakePhoto Invalid message header\n");
        return -1;
    }

    if(pic_num > 10)
    {
        pic_num = 1;
    }

    char file_path[256] = {0};
    snprintf(file_path, sizeof(file_path), "%s%d.jpg", TAKE_PHOTO_TMP_FILE, pic_num);
    int ret = cm_video_take_photo_save_to_file(file_path);
    uint8_t response[1024] = {0};
    uint16_t index = 0;
    response[index++] = 0xAA;
    response[index++] = 0x5A;
    response[index++] = 0x06;
    response[index++] = 0x00;
    response[index++] = ret;
    send_msg_resp(response, index);
    pic_num++;
    return index;
}

int HandelCameraRoot(uint8_t *msg_buf, uint16_t msg_len)
{
    if (msg_buf == NULL || msg_len < 4) {
        LOGD("CameraRoot Invalid message buffer\n");
        return -1;
    }
    if (msg_buf[0] != 0xAA || msg_buf[1] != 0x5A || msg_buf[2] != 0x05 || msg_buf[3] != 0x00) {
        LOGD("CameraRoot Invalid message header\n");
        return -1;
    }

    sync();
    int ret = system("reboot");

    uint8_t response[1024] = {0};
    uint16_t index = 0;
    response[index++] = 0xAA;
    response[index++] = 0x5A;
    response[index++] = 0x05;
    response[index++] = 0x00;
    response[index++] = ret;
    send_msg_resp(response, index);
    return index;
}
