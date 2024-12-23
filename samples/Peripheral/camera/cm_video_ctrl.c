/**
 * @file cm_video_ctrl.c
 * @author jmdvirus
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "cm_video_ctrl.h"
#include "cm_video_interface.h"

static const CMVideoImpl *cm_video_impl_list[] = {
    &cm_video_impl_t23,
    NULL,
};

static const CMVideoImpl *CurVideoImpl = NULL;
static CMVideoContext ctx;
static int is_initialized = 0;
static pthread_mutex_t video_mutex = PTHREAD_MUTEX_INITIALIZER;
#define VIDEO_TAG "VIDEO"


const CMVideoImpl *cm_video_impl_find(const char *type)
{
    CMVideoImpl *impl = NULL;
    int          i    = 0;
    while ((impl = (CMVideoImpl *)cm_video_impl_list[i++]) != NULL)
    {
        if (strlen(impl->name) == strlen(type) && strcmp(impl->name, type) == 0)
        {
            return impl;
        }
    }
    return NULL;
}
int cm_video_impl_init(const char *type)
{
    if (is_initialized) {
        printf("[%s] Already initialized\n", VIDEO_TAG);
        return 0;
    }

    if (type == NULL) {
        printf("[%s] Error: type is NULL\n", VIDEO_TAG);
        return -1;
    }

    CurVideoImpl = cm_video_impl_find(type);
    if (CurVideoImpl == NULL) {
        printf("[%s] Error: failed to find implementation\n", VIDEO_TAG);
        return -1;
    }

    // Initialize context
    memset(&ctx, 0, sizeof(ctx));

    // Open device
    if (CurVideoImpl->open && CurVideoImpl->open(&ctx, "") != 0) {
        printf("[%s] Error: failed to open device\n", VIDEO_TAG);
        return -1;
    }

    // Start device
    if (CurVideoImpl->start && CurVideoImpl->start(&ctx) != 0) {
        printf("[%s] Error: failed to start device\n", VIDEO_TAG);
        if (CurVideoImpl->close) {
            CurVideoImpl->close(&ctx);
        }
        return -1;
    }

    is_initialized = 1;
    return 0;
}



/**
 * @brief Take photo and save to file
 */
int cm_video_take_photo_save_to_file(const char *file_path)
{
    int ret = 0;

    if (pthread_mutex_lock(&video_mutex) != 0) {
        printf("[%s] Failed to acquire mutex\n", VIDEO_TAG);
        return -1;
    }

    do {
        if (!is_initialized || !CurVideoImpl) {
            printf("[%s] Error: Not initialized\n", VIDEO_TAG);
            ret = -1;
            break;
        }

        if (file_path == NULL) {
            printf("[%s] Error: Invalid parameters\n", VIDEO_TAG);
            ret = -1;
            break;
        }

        // 分配缓冲区
        CMVideoBuf buf = {0};

        // 读取图像数据
        if (CurVideoImpl->read == NULL || 
            CurVideoImpl->read(&ctx, &buf) != 0) {
            printf("[%s] Error: Failed to read image data\n", VIDEO_TAG);
            ret = -1;
            break;
        }

        // 临时释放锁进行文件操作
        pthread_mutex_unlock(&video_mutex);

        // 保存文件
        FILE *fp = fopen(file_path, "wb");
        if (fp == NULL) {
            printf("[%s] Error: Failed to open file: %s\n", 
                   VIDEO_TAG, file_path);
            return -1;
        }

        size_t written = fwrite(buf.start, 1, buf.length, fp);
        fclose(fp);

        // 重新获取锁以检查结果
        if (pthread_mutex_lock(&video_mutex) != 0) {
            printf("[%s] Failed to reacquire mutex\n", VIDEO_TAG);
            return -1;
        }

        if (written != buf.length) {
            printf("[%s] Error: Failed to write file\n", VIDEO_TAG);
            ret = -1;
            break;
        }

        printf("[%s] Successfully saved photo to: %s\n", 
               VIDEO_TAG, file_path);
    } while (0);

    pthread_mutex_unlock(&video_mutex);
    return ret;
}




void cm_video_impl_deinit(void)
{
    if (!is_initialized || CurVideoImpl == NULL) {
        return;
    }

    // 停止设备
    if (CurVideoImpl->stop) {
        CurVideoImpl->stop(&ctx);
    }

    // 关闭设备
    if (CurVideoImpl->close) {
        CurVideoImpl->close(&ctx);
    }

    is_initialized = 0;
    CurVideoImpl = NULL;
    memset(&ctx, 0, sizeof(ctx));
}
