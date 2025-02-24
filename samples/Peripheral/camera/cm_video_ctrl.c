/**
 * @file cm_video_ctrl.c
 * @author jmdvirus
 */
#define LOG_TAG "CM_VIDEO_CTRL"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "cm_video_ctrl.h"
#include "cm_video_interface.h"
#include "elog.h"

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
        log_w("Video already initialized");
        return 0;
    }

    if (type == NULL) {
        log_e("video type is NULL");
        return -1;
    }

    CurVideoImpl = cm_video_impl_find(type);
    if (CurVideoImpl == NULL) {
        log_e("can not find video implementation");
        return -1;
    }

    // Initialize context
    memset(&ctx, 0, sizeof(ctx));

    // Open device
    if (CurVideoImpl->open && CurVideoImpl->open(&ctx, "") != 0) {
        log_e("failed to open device");
        return -1;
    }

    // Start device
    if (CurVideoImpl->start && CurVideoImpl->start(&ctx) != 0) {
        log_e("failed to start device");
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
        log_e("Failed to acquire mutex");
        return -1;
    }

    do {
        if (!is_initialized || !CurVideoImpl) {
            log_e("Not initialized");
            ret = -1;
            break;
        }

        if (file_path == NULL) {
            log_e("Invalid parameters");
            ret = -1;
            break;
        }

        // 分配缓冲区
        CMVideoBuf buf = {0};

        // 读取图像数据
        if (CurVideoImpl->read == NULL || 
            CurVideoImpl->read(&ctx, &buf) != 0) {
            log_e("Failed to read image data");
            ret = -1;
            break;
        }

        // 临时释放锁进行文件操作
        pthread_mutex_unlock(&video_mutex);

        // 保存文件
        FILE *fp = fopen(file_path, "wb");
        if (fp == NULL) {
            log_e("Failed to open file: %s", file_path);
            return -1;
        }

        size_t written = fwrite(buf.start, 1, buf.length, fp);
        fclose(fp);

        // 重新获取锁以检查结果
        if (pthread_mutex_lock(&video_mutex) != 0) {
            log_e("Failed to reacquire mutex");
            return -1;
        }

        if (written != buf.length) {
            log_e("Failed to write file");
            ret = -1;
            break;
        }

        log_i("Successfully saved photo to: %s", file_path);
    } while (0);

    pthread_mutex_unlock(&video_mutex);
    return ret;
}



int cm_video_ctrl(CMVideoCommand cmd, void *arg)
{
    int ret = 0;
    if (CurVideoImpl == NULL) {
        return -1;
    }

    if (pthread_mutex_lock(&video_mutex) != 0) {
        log_e("Failed to acquire mutex");
        return -1;
    }

    if (CurVideoImpl->ctrl) {
        // 直接传递完整参数结构
        ret = CurVideoImpl->ctrl(&ctx, cmd, arg);
        if (ret < 0)
        {
            log_e("Failed to control video");
        }
        else
        {
            log_i("Successfully control video");
        }
    }

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
