#include "ProtocolPort.h"
#include "cm_video_ctrl.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>


uint8_t get_image_seq()
{
    static uint8_t image_seq = 0;
    image_seq++;
    return image_seq;
}


uint8_t request_take_photo(char *file)
{
    return cm_video_take_photo_save_to_file(file);
}
/**
 * 检查并创建目录
 * @param dir_path 目录路径
 * @return 成功返回 0，失败返回 -1 并设置 errno
 */
static int check_and_create_dir(const char *dir_path)
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
int Protocol_Init()
{
    check_and_create_dir(ACTIVE_TRIGGER_PHOTO_FILE_DIR);
    check_and_create_dir(GYRO_TRIGGER_PHOTO_FILE_DIR);
}   