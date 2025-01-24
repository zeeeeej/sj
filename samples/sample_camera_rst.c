
#include <stdio.h>
#include <unistd.h>

#include "cm_video_ctrl.h"
#include "imp/imp_log.h"
#include "door_detect.h"
#include "cm_common.h"
#include "module_config.h"
#include <stdlib.h>
#include "wdt.h"
//echo V > /dev/watchdog
#include "door_detect.h"
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include "self_check.h"
#include "circular_log.h"
#include "cli_command.h"
#include "debug_logger.h"
#include "MsgDispatcher.h"
#include "ProtocolPort.h"
#include <sys/stat.h>
#include "Log_init.h"
#include <elog.h>
#define TAG_NAME "[MAIN]"
static int b_exited = 0;
static pthread_t hmi_srv_tid = 0;


char *version = "jml_V1.0.1";

    
#define VERSION_FILE "/system/etc/version"
#define LOG_FILE "/system/log/t23.log"
#define LOG_DIR "/system/log"
#define LOG_FILE_SIZE 1024 * 30


static int write_version_to_file(void) {
    FILE *fp = fopen(VERSION_FILE, "w");  // 使用"w"模式会清除原有内容
    if (fp == NULL) {
        log_write(LOG_ERROR, TAG_NAME, "Failed to open version file: %s", strerror(errno));
        return -1;
    }
    // 写入版本信息
    fprintf(fp, "Version: %s\n", version);
    fclose(fp);
    return 0;
}


int main(int argc, char *argv[])
{   

    my_log_init();
    log_i("startup [%s:%s] Version [%s]", __DATE__, __TIME__, version);
    /*写入版本信息到文件*/
    if (write_version_to_file() < 0) {
        log_e("Failed to write version information to file");
    } else {
        log_i("Version information written to %s", VERSION_FILE);
    }


    /*启动看门狗*/
    int wdt_disable = (access("/system/etc/wdt_disable", F_OK) == 0);
    if (!wdt_disable)
    {
        wdt_enable();
        wdt_set_timeout(15);
        int timeout = wdt_get_timeout();
        log_i("wdt timeout set to [%d]\n", timeout);
    }

    Protocol_Init();
    Cam485ProtocolInit();

    /*初始化video模块*/
    cm_video_impl_init("t23");
    self_check_start();
    cm_video_impl_deinit();
    int ret1;


    
    // wind_connect_up_start();
    door_init();
    



    // printf("create hmi service thread\n");
    // extern void *hmi_service_thread(void *args);
    // int ret = pthread_create(&hmi_srv_tid, NULL, hmi_service_thread, NULL);
    // if (ret != 0)
    // {
    //     fprintf(stderr, "Error creating thread: %s\n", strerror(ret));

    //     return 1; 
    // }
    // else
    // {
    //     printf("Thread created successfully.\n");
    // }


    // printf("create hmi service thread\n");
    // extern void *hmi_service_thread(void *args);
    // int ret = pthread_create(&hmi_srv_tid, NULL, hmi_service_thread, NULL);
    // if (ret != 0)
    // {
    //     fprintf(stderr, "Error creating thread: %s\n", strerror(ret));

    //     return 1; 
    // }
    // else
    // {
    //     printf("Thread created successfully.\n");
    // }
    while (!b_exited)
    {
        if (!wdt_disable)
        {
            wdt_keep_alive();
        }
        usleep(20000);
    }

    door_deinit();


    return 0;
}
