#include "Cam485Protocol.h"
#include "Cam485ProtocolCommon.h"
#include "MsgDispatherPort.h"
#include "cm_video_ctrl.h"
#include "door_detect.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>


static pthread_mutex_t seq_mutex = PTHREAD_MUTEX_INITIALIZER;


/*获取图片序列号*/
uint8_t get_image_seq()
{
    static uint8_t image_seq = 0;

    // 加锁
    pthread_mutex_lock(&seq_mutex);

    // 递增image_seq
    image_seq++;

    // 检查是否达到255，如果达到则回退到0
    if (image_seq > 255) {
        image_seq = 0;
    }

    // 解锁
    pthread_mutex_unlock(&seq_mutex);

    return image_seq;
}

uint8_t request_take_photo(char *file)
{
    return cm_video_take_photo_save_to_file(file);
}

uint8_t query_door_status()
{
    return get_door_status();
}

int check_and_create_dir(const char *dir_path)
{
    if (access(dir_path, F_OK) == -1) {
        // 目录不存在，尝试创建
        if (mkdir(dir_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1) {
            perror("mkdir failed");
            return -1;
        }
        printf("Directory created: %s\n", dir_path);
    } else {
        printf("Directory exists: %s\n", dir_path);
    }
    return 0;
}

int remove_dir(const char *dir_path) {
    char command[256];
    snprintf(command, sizeof(command), "rm -rf %s", dir_path);

    // 使用system命令执行删除操作
    if (system(command) == -1) {
        perror("system failed");
        return -1;
    }
    return 0;
}

int Protocol_Init()
{
    check_and_create_dir(ACTIVE_TRIGGER_PHOTO_FILE_DIR);
    check_and_create_dir(GYRO_TRIGGER_PHOTO_FILE_DIR);
    Cam485ProtocolInit();
}   