#include <stdio.h>
#include <unistd.h>

#include "cm_video_ctrl.h"
#include "imp/imp_log.h"
#include "door_detect.h"
#include "cm_common.h"
#include "wdt.h"
//echo V > /dev/watchdog
#include "door_detect.h"
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include "self_check.h"
#include "circular_log.h"
#include "cli.h"
#include "debug_logger.h"
#include "Cam485Protocol.h"
#include "cm_config.h"
#include <sys/stat.h>
#include "Log_init.h"
#include "elog.h"
#include "cm_config.h"
#include "ParseIni.h"
#include "pwm_ctrl.h"
#define TAG_NAME   "[MAIN]"
static int b_exited = 0;

#define LOG_FILE "/system/log/t23.log"
#define LOG_DIR "/system/log"
#define LOG_FILE_SIZE 1024 * 30

char *version = "1.0.2";

// 在头文件后添加宏定义
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

// 通过Makefile传递的编译信息
#ifndef GIT_VERSION
#define GIT_VERSION "unknown"
#endif
#ifndef BUILD_TIMESTAMP
#define BUILD_TIMESTAMP "unavailable"
#endif
#ifndef COMPILER_FLAGS
#define COMPILER_FLAGS "default"
#endif

// 版本信息结构体
static const char* COMPILE_INFO =
    "Firmware Version: %s\n"
    "Build Time: %s %s\n"
    "Compiler: %s\n"
    "Flags: %s\n"
    "Git Commit: %s\n"
    "Architecture: %s\n";

static void print_build_info(void) {
    log_i(COMPILE_INFO,
          version,
          __DATE__, __TIME__,
          __VERSION__,
          TOSTRING(COMPILER_FLAGS),
          GIT_VERSION,
#if defined(__arm__)
          "ARM"
#elif defined(__aarch64__)
          "ARM64"
#else
          "Unknown"
#endif
    );
}

static int save_version(const char *version_str) {
    if (version_str == NULL) {
        log_e("Version string is NULL");
        return -1;
    }
    // 调用函数保存版本号
    int ret = Set_g_firmwareVersion(version);
    if (ret != 0) {
        log_e("Set firmware version failed");
        return -1;
    }

    log_i("Successfully saved version: %s", version_str);
    return 0;
}

int main(int argc, char *argv[])
{   
    my_log_init();
    print_build_info();  // 新增编译信息打印
    // linux_cmd_init();
    log_i("startup [%s:%s] Version [%s]", __DATE__, __TIME__, version);
    /*解析配置文件*/    
    parse_ini();
    generate_sn();
    /*写入版本信息到配置文件*/
    if (save_version(version) < 0) {
        log_e("Failed to write version information to file");
    } 
    /*初始化命令行*/
    // command_init();
    /*启动看门狗*/
    int wdt_disable = (access("/system/etc/wdt_disable", F_OK) == 0);
    if (!wdt_disable)
    {
        wdt_enable();
        wdt_set_timeout(15);
        int timeout = wdt_get_timeout();
        log_i("wdt timeout set to [%d]\n", timeout);
    }



    /*初始化视频模块*/
    cm_video_impl_init("t23");

    // run_pwm();
    /*开始自检*/
    // self_check_start();
    /*485协议初始化*/
    Protocol_Init();
    /*门开关检测初始化*/
    door_detect_init();
    
    while (!b_exited)
    {
        if (!wdt_disable)
        {
            wdt_keep_alive();
        }
        usleep(20000);
    }

    door_detect_deinit();

    return 0;
}
