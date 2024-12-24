
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

#include "door_detect.h"
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include "self_check.h"
#include "MessageDispatcher.h"
#include "circular_log.h"
#include "cli_command.h"
// #include "MessageDispaterPort.h"
#define TAG_NAME "[MAIN]"

static int b_exited = 0;
static pthread_t hmi_srv_tid = 0;
/***********注意版本号要更新两处地方************/
/***********注意版本号要更新两处地方************/
/***********注意版本号要更新两处地方************/
/***********注意版本号要更新两处地方************/

#define VERSION "signel_display_t23_1.0.1"
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
int main(int argc, char *argv[])
{   
    printf("init log system\n");
    if (log_init("/var/log/t23.log", 1024 * 5) < 0) {
        fprintf(stderr, "Failed to initialize log system: %s\n", strerror(errno));
        return -1;
    }
    log_write(LOG_DEBUG, TAG_NAME, "startup [%s:%s] Version [%s]\n", __DATE__, __TIME__, "signel_display_t23_1.0.1");


    // if (cli_init() != 0) {
    //     log_write(LOG_ERROR, TAG_NAME, "Failed to initialize CLI");
    //     return -1;
    // }

    // // 注册版本查询命令
    // static cli_command_t version_cmd = {
    //     .name = "version",
    //     .help = "Show system version information",
    //     .func = cmd_version
    // };

    // if (cli_register_command(&version_cmd) != 0) {
    //     log_write(LOG_ERROR, TAG_NAME, "Failed to register version command");
    //     return -1;
    // }

    // if (cli_start() != 0) {
    //     log_write(LOG_ERROR, TAG_NAME, "Failed to start CLI");
    //     return -1;
    // }


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
    /*self check*/
    // t23_self_check_start();

    cm_config_load();
    // wind_connect_init_in_thread();
    door_init();
    wind_connect_up_start();
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
