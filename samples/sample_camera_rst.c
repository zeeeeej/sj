
#include <stdio.h>
#include <unistd.h>

#include "cm_video_ctrl.h"
#include "imp/imp_log.h"
#include "door_detect.h"
#include "cm_common.h"
#include "module_config.h"
#include <module_ucamera_control.h>
#include <stdlib.h>

#include "cm_config.h"
#include "wind_global.h"
#include "wind_connect.h"
#include "wdt.h"
#include "wind_connect_up.h"
//echo V > /dev/watchdog
#include "door_detect.h"
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include "self_check.h"
#include "MessageDispatcher.h"
#include "circular_log.h"
#include "cli_command.h"
#include "debug_logger.h"
#include "MsgDispatcher.h"
#include "ProtocolPort.h"
#define TAG_NAME "[MAIN]"
static int b_exited = 0;
static pthread_t hmi_srv_tid = 0;


char *version = "signel_display_t23_1.0.4";
/*优化上位机出图效率
作以下改进：
    1、使用二进制模式来传图
    2、陀螺仪自检的时候等校准
    hmi_server会等待陀螺仪校准完成，执行开关门检测判断
    wind_conn会先启动*/
    
#define VERSION_FILE "/system/etc/version"
#define LOG_FILE "/system/log/t23.log"
#define LOG_DIR "/system/log"
#define LOG_FILE_SIZE 1024 * 30
#define BUILD_DATE __DATE__
#define BUILD_TIME __TIME__
static int cmd_version(int argc, char *argv[]) {
    printf("\nSystem Version Information:\n");
    printf("------------------------------------------\n");
    printf("Version: %s\n", VERSION);
    printf("Build Date: %s\n", BUILD_DATE);
    printf("Build Time: %s\n", BUILD_TIME);
    printf("------------------------------------------\n");
    return 0;
}
static int write_version_to_file(void) {
    FILE *fp = fopen(VERSION_FILE, "w");  // 使用"w"模式会清除原有内容
    if (fp == NULL) {
        log_write(LOG_ERROR, TAG_NAME, "Failed to open version file: %s", strerror(errno));
        return -1;
    }

    // 写入版本信息
    fprintf(fp, "Version: %s\n", version);
    // fprintf(fp, "Build Date: %s\n", BUILD_DATE);
    // fprintf(fp, "Build Time: %s\n", BUILD_TIME);

    fclose(fp);
    return 0;
}
int create_directory(const char* path) {
    // 尝试创建目录，权限设置为 0755
    if (mkdir(path, 0755) == 0) {
        printf("Directory created: %s\n", path);
        return 0;
    } else {
        // 如果目录已经存在，mkdir 会返回 -1 并设置 errno 为 EEXIST
        if (errno == EEXIST) {
            return 0;  // 目录已经存在，不是错误
        } else {
            perror("Failed to create directory");
            return -1;
        }
    }
}

int main(int argc, char *argv[])
{   
    printf("hello world !\n");
    printf("init log system\n");
    /*===========初始化日志系统===========*/
    if (create_directory(LOG_DIR) != 0) {
        return -1;
    }
    if (log_init(LOG_FILE, LOG_FILE_SIZE) < 0) {
        fprintf(stderr, "Failed to initialize log system\n");
        return -1;
    }
    log_write(LOG_DEBUG, TAG_NAME, "startup [%s:%s] Version [%s]\n", __DATE__, __TIME__, version);

    // 写入版本信息到文件
    if (write_version_to_file() < 0) {
        log_write(LOG_ERROR, TAG_NAME, "Failed to write version information to file");
    } else {
        log_write(LOG_INFO, TAG_NAME, "Version information written to %s", VERSION_FILE);
    }
    /*===========初始化日志系统===========*/


    /*===========初始化debug打印系统===========*/
    debug_logger_init();
    /*===========初始化debug打印系统===========*/

    
    int wdt_disable = (access("/system/etc/wdt_disable", F_OK) == 0);
    if (!wdt_disable)
    {
        wdt_enable();
        wdt_set_timeout(15);
        int timeout = wdt_get_timeout();
        LOGD("wdt timeout set to [%d]\n", timeout);
    }

    /*初始化video模块*/
    cm_video_impl_init("t23");
    self_check_start();
    cm_video_impl_deinit();
    int ret1;


    cm_config_load();
    Protocol_Init();
    Cam485ProtocolInit();
    // wind_connect_up_start();
    door_init();
    
    // printf("create message dispatcher thread\n");
    // ret1 = create_message_dispatcher_thread();
    // if(ret1 != 0)
    // {
    //     printf("create message dispatcher thread failed\n");
    //     return 1;
    // }


    printf("create hmi service thread\n");
    extern void *hmi_service_thread(void *args);
    int ret = pthread_create(&hmi_srv_tid, NULL, hmi_service_thread, NULL);
    if (ret != 0)
    {
        fprintf(stderr, "Error creating thread: %s\n", strerror(ret));

        return 1; 
    }
    else
    {
        printf("Thread created successfully.\n");
    }
    while (!b_exited)
    {
        if (!wdt_disable)
        {
            wdt_keep_alive();
        }
        usleep(20000);
    }

    door_deinit();
    // wind_connect_deinit();
    wind_connect_up_stop();

    return 0;
}
