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
    return index;
}

int HandlePhotoRequest(const uint8_t *msg_buf, uint16_t msg_len)
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
    free(data);
    free(base64_buffer);
    return ret;
}
