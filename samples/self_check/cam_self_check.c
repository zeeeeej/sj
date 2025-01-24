#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "cam_self_check.h"
#include "cm_video_interface.h"
#include "cm_video_ctrl.h"
#include "cm_utils.h"
#include "cm_common.h"
#include "cm_conf.h"
#include "errno.h"
// #include "cm_log.h"

unsigned long long cm_tick_milli();
#define CHECK_JPEG_FILE "/tmp/selfcheck/selfcheck.jpg"
#define CHECK_JPEG_DIR "/tmp/selfcheck"
#define CHECK_CAMERA_TYPE "t23"
#define SELF_CHECK_TIME 30
#define MIN_IMAGE_SIZE 10000
#define FILTER_FRAME_COUNT 5
#define SLEEP_INTERVAL_US 20000 // 20ms


/*todo : 图片质量检测 根据mcu类型判断是否需要iqva检测*/
/*todo : 读取图片分辨率设置*/
int cam_self_check_start(SelfCheckProperty *checkProperty)
{
    int ret;
    const char *checkJpeg = CHECK_JPEG_FILE;
    struct stat file_stat;
    const char *dir  = CHECK_JPEG_DIR;
    LOGD(" ***Camera Self Check Started*** ");
    LOGD("Check file path: %s", checkJpeg);
    ret = cm_video_impl_init(CHECK_CAMERA_TYPE);
    if (ret != 0)
    {
        LOGD("ERROR: Camera init failed with code: %d", ret);
        checkProperty->result = false;
        goto exit;
    }
    // 添加目录检查和创建
    LOGD("Checking directory: %s", dir);
    if (access(dir, F_OK) != 0)
    {
        LOGD("Directory does not exist, attempting to create...");
        if (mkdir(dir, 0777) != 0)
        {
            LOGD("ERROR: Failed to create directory: %s (errno: %d: %s)",
                 dir, errno, strerror(errno));
            checkProperty->result = false;
            goto exit;
        }
        LOGD("Directory created successfully");
    }

    // 检查目录权限
    if (access(dir, W_OK) != 0)
    {
        LOGD("ERROR: No write permission to directory: %s (errno: %d: %s)",
             dir, errno, strerror(errno));
        checkProperty->result = false;
        goto exit;
    }
    LOGD("Directory permissions OK");

    // 初始化检查属性
    checkProperty->startTime = cm_gettime_milli();
    checkProperty->tickStart = cm_tick_milli();
    checkProperty->result = true;
    // checkProperty->resultStrin   g = YQSL_ERROR_NAME_OK;
    LOGD("Self check initialized at time: %llu", checkProperty->startTime);

    // 清理旧文件
    if (access(checkJpeg, F_OK) == 0)
    {
        LOGD("Removing existing check file");
        remove(checkJpeg);
    }


    // 过滤前几帧
    LOGD("Filtering first %d frames...", FILTER_FRAME_COUNT);
    for (int i = 0; i < FILTER_FRAME_COUNT; i++)
    {
        ret = cm_video_take_photo_save_to_file(CHECK_JPEG_FILE);
        if (ret != 0)
        {
            LOGD("Warning: Frame %d filtering failed with code: %d", i, ret);
        }
    }

    // 尝试捕获有效图片
    LOGD("Starting image capture loop (max attempts: %d)", SELF_CHECK_TIME);
    bool capture_success = false;
    for (int count = 0; count < SELF_CHECK_TIME && !capture_success; count++)
    {
        LOGD("Capture attempt %d/%d", count + 1, SELF_CHECK_TIME);
        ret = cm_video_take_photo_save_to_file(CHECK_JPEG_FILE);
        if (ret == 0)
        {
            struct stat file_stat;
            if (stat(CHECK_JPEG_FILE, &file_stat) == 0)
            {
                LOGD("Frame read successful - Size: %ld bytes (min required: %d)",
                     file_stat.st_size, MIN_IMAGE_SIZE);
                
                if (file_stat.st_size > MIN_IMAGE_SIZE)
                {
                    LOGD("SUCCESS: Image captured and saved successfully");
                    capture_success = true;
                    checkProperty->result = true;
                    break;
                }
                else
                {
                    LOGD("ERROR: Frame too small (%ld bytes)", file_stat.st_size);
                    checkProperty->result = false;
                }
            }
            else
            {
                LOGD("ERROR: Frame too small (%ld bytes)", file_stat.st_size);
                checkProperty->result = false;
            }
            usleep(SLEEP_INTERVAL_US);
        }
        else
        {
            LOGD("ERROR: Frame read failed with code: %d", ret);
        }
    }
exit:

    checkProperty->endTime = cm_gettime_milli();
    checkProperty->tickEnd = cm_tick_milli();
    // 在退出时添加时间统计
    unsigned long long duration = checkProperty->endTime - checkProperty->startTime;
    LOGD(" *** Camera Self Check Completed ***");
    LOGD("Duration: %llu ms", duration);
    LOGD("Final result: %s", checkProperty->result ? "SUCCESS" : "FAILED");
    LOGD("Check file size: %ld bytes", file_stat.st_size);
    LOGD(" *** End of Camera Self Check ***");

    /*删除自检照片*/
    
    return checkProperty->result ? 0 : -1;
}