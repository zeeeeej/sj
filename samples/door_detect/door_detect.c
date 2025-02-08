#include "stdint.h"
// #include "cm_mpu_ctrl.h"
#include <imp/imp_log.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
// #include "cm_config.h"
#include "cm_video_ctrl.h"
#include "cm_utils.h"
#include "cm_conf.h"
#include <signal.h>
#include <unistd.h> // for access()
#include "cm_common.h"
#include "yq_mpu.h"
#include "mpu_ctrl.h"
#include "circular_log.h"
#include "ImageInfoList.h"
static char debug_flag = 0;
static char *TAG = "door detect";
static float debug_reference_ang = 0.0;
static volatile int debug_print_enabled = 1; // 0: 不打印，1: 打印

static float min_capture_angle = 55.0f;
static float max_capture_angle = 65.0f;
static char door_status = 0;
static int direction = 0;

#define CM_CONFIG_FILE "/system/etc/cm_config.ini"
static CMConfig cm_config_local[100];
static char *TAG_NAME = "door detect";
#define DEBUG_FLAG_FILE "/tmp/door_debug_enable"


/****************config param ******************/
// 当次开门缓存下来的信息
typedef struct {
    float image_angle;
    char image_path[256];
    unsigned long long open_count;
    int array_index;
} DoorInfoItem;

typedef struct {
    float angle_open;
    float angle_close;
    float angle_max;
    int door_open_timeout;
} DoorConfigArgs;

typedef enum
{
    DOOR_IDLE,
    DOOR_SHAKE,
    DOOR_OPEN
} door_act_e;

typedef enum
{
    STAT_IDLE,
    STAT_CLOSE,
    STAT_ACTION,
    STAT_ABNORMAL
} door_stat_e;

/*==== soruce code  ===*/
enum
{
    DOOR_CLOSE = 0,
    DOOR_OPENED,
};

typedef struct
{
    float angle_start;
    float angle_door_open;
    float angle_door_close;

    // 上一次的角度, 用于过滤抖动
    float angle_last;
    unsigned long long last_time;
} AngleTolerance;

typedef struct
{
    unsigned long long last_open_time;
} DoorOpenTimeout;

typedef struct
{
    int status;
    int inited;
    DoorConfigArgs *door_config;

    CMVideoImpl *video_impl;
    CMVideoContext ctx;

    // 当前角度
    float angle_now;
    // 开门次数
    unsigned long long open_count;

    // 开门后的最大角度
    float max_angle;
    float angle_last_valid;
    float angle_current_valid;
    AngleTolerance angle_tolerant;
    DoorOpenTimeout door_open_timeout;
    uint8_t image_seq;
    CMVideoBuf buf;
} DoorDetect;

static DoorDetect s_door;

static door_act_e debug_action = DOOR_IDLE;
static int debug_state = STAT_IDLE;

/*根据实际门的角度来修改*/
#define DOOE_DIRECTION 2
#define CAPTURE_ANGLE 4.0
typedef struct
{
    char running;
    int handle_status; // door_stat_e
    int door_status;   // door_stat_e
    void (*open_cb)(void *args);
    void (*close_cb)(void *args);
    void (*photo_cb)(void *args);
    void (*touch_cb)(void *args);
    void (*release_cb)(void *args);
} hmi_srv_t;

hmi_srv_t hmi_srv;


static int door_close()
{
    DoorInfoItem item;
    item.open_count = s_door.open_count;
    item.image_angle = s_door.max_angle;
    item.array_index = item.open_count % 5;
    LOGD("door close open count [%llu] angle now [%f] array_index [%d]\n",
         item.open_count, item.image_angle, item.array_index);
    // wind_process_manager_status_changed(PROCESS_STATUS_IDLE);
    // wind_connect_up_door_close(&item);
    return 0;
}


static int door_take_photo()
{
    DoorInfoItem item;
    item.open_count = s_door.open_count;
    item.image_angle = s_door.angle_current_valid;
    item.array_index = item.open_count % 5;
    // wind_connect_up_take_photo(&item);
    return 0;
}


static int door_open(float angle)
{
    // wind_process_manager_status_changed(PROCESS_STATUS_DOOR_OPEN);
    // wind_connect_up_door_open(s_door.open_count, angle);
    return 0;
}

static int image_cache()
{
    /*移除上一个文件*/

    int ret;
    char old_path[256] = {0};
    snprintf(old_path, sizeof(old_path), "/tmp/gyro_trigger/image_%d_%d.jpg", s_door.image_seq, (int)s_door.angle_last_valid);
    char cmd_remove_path[256] = {0};
    snprintf(cmd_remove_path, sizeof(cmd_remove_path), "rm -rf %s", old_path);
    LOGD("remove [%s]\n", cmd_remove_path);
    system(cmd_remove_path);
    deleteNodeByFileName(old_path);
    DoorInfoItem item;
    item.open_count = s_door.open_count;
    item.image_angle = s_door.angle_current_valid;
    item.array_index = item.open_count % 5;
    snprintf(item.image_path, sizeof(item.image_path), "/tmp/gyro_trigger/image_%d_%d.jpg",
             s_door.image_seq, (int)item.image_angle);
    ret = cm_video_take_photo_save_to_file(item.image_path);
    if(ret != 0 )
    {
        log_write(LOG_ERROR, TAG, "image cache fail");
    }
    generate_image_info(item.image_path);
    /*更新上一个角度为当前角度*/
    s_door.angle_last_valid = s_door.angle_current_valid;
    return 0;
}

unsigned long long cm_tick_milli()
{
    struct timespec ts;

    int ret = clock_gettime(CLOCK_MONOTONIC, &ts);
    if (ret < 0)
    {
        return 0;
    }
    return ((unsigned long long)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000));
}

static uint8_t door_open_detect(float diff, float last_ang, float *reference_yaw, float ang)
{
    static uint32_t   dyn_cnt = 0, stable_cnt = 0, stable_flag = 0;
    static door_act_e action = DOOR_IDLE;

    if (debug_flag)
    {
        printf(
            "timestamp[%lld]: diff [%f] last [%f] reference [%f] ang [%f] stable_cnt [%d] dyn_cnt [%d] stable_flag [%d] action [%d]\n",
            cm_tick_milli(), diff, last_ang, *reference_yaw, ang, stable_cnt, dyn_cnt, stable_flag, action);
    }
    switch (action)
    {
    case DOOR_IDLE:
        if (fabs(diff) < 0.1)
        {
            stable_cnt++;

            if (stable_cnt > 10)
            {                              // 稳定约200ms
                dyn_cnt = 0;
                *reference_yaw = last_ang; // 记录下门动作的初始角度值，应用层上消除陀螺仪零漂
                stable_cnt     = 0;
                stable_flag    = 1;
            }
        }
        else
        {
            dyn_cnt++;
            stable_cnt = 0;
            if (dyn_cnt == 5)
            {
                dyn_cnt = 0;
                if (stable_flag)
                { // 抖动前稳定过，防止缓慢开门小角度重复触发
                    LOGD("door detect [IDLE -> SHAKE]");
                    action      = DOOR_SHAKE;
                    stable_flag = 0;
                }
            }
        }
        break;
    case DOOR_SHAKE:
        dyn_cnt++;
        if (ang > 20)
        { // 开门角度阈值
            action = DOOR_OPEN;
            LOGD("door detect [SHAKE -> OPEN]");
            dyn_cnt = 0;
        }

        if (dyn_cnt > 200 || ang < -2.0)
        { // 约1s无开门动作，则判定为抖动；角度为负则为关门动作
            action = DOOR_IDLE;
            LOGD("door detect [SHAKE -> IDLE]");
            dyn_cnt = 0;
            return 2;
        }
        break;
    case DOOR_OPEN:
        action = DOOR_IDLE;
        LOGD("door detect [OPEN -> IDLE]");
        break;
    }

    debug_action        = action;
    debug_reference_ang = *reference_yaw;
    return (action == DOOR_OPEN) ? 1 : 0;
}


static uint8_t door_close_detect(float ang, float diff, long openTime)
{
    static uint16_t cnt  = 0;
    static uint16_t cnt2 = 0;
    if (debug_flag)
    {
        printf("cnt [%d] cnt2 [%d] ang [%f] diff [%f]\n", cnt, cnt2, ang, diff);
    }

    // 判断关门情况一:开门角度小于4度，且实时角度偏差小于0.1度，持续100毫秒
    if ((ang < 6.0) && (fabs(diff) < 0.05))
    {
        cnt++;
    }
    else
    {
        cnt = 0;
    }

    if (cnt >= 10)
    { //静止约200ms
        cnt = 0;
        return 1;
    }

    // 判断关门情况三: 开门角度小于10度，且实时角度偏差小于0.1度，持续1秒
    if ((ang < 10.0) && (fabs(diff) < 0.1))
    {
        cnt2++;
    }
    else
    {
        cnt2 = 0;
    }

    if (cnt2 >= 50)
    { // 静止约1秒
        cnt2 = 0;
        return 1;
    }

    // 判断关门情况二: 长时间未发现关门动作（可能在上货、陀螺仪数据异常）*/
    if (cm_tick_milli() - openTime > 5 * 60 * 1000)
    {
        LOGD("Door long time no close");
        return 2;
    }
    return 0;
}

static uint8_t gyroscope_stable(float diff)
{
    static uint8_t stable_cnt = 0;
    if (fabs(diff) < 0.01)
    {
        stable_cnt++;
    }
    else
    {
        stable_cnt = 0;
    }

    if (stable_cnt < 30)
    {
        return 0;
    }
    else
    {
        stable_cnt = 0;
        return 1;
    }
}

/**/
void check_door_direction()
{
}

void door_status_detect(float yaw, char flag, float capture_angle)
{
    static float last_yaw = 0;
    static float diff_yaw = 0;
    static float reference_yaw = 0;
    float angle = 0;
    static float max_angle = 0;
    static uint8_t take_photo_on = 0;
    // static image_cache_counter = 0;
    int ret = 0;
    static long open_door_time = 0;

    /*表示是否第一次进入开门状态检测*/
    static uint8_t door_status_flag = 0;

    diff_yaw = yaw - last_yaw;
    angle = yaw - reference_yaw; // 实时角度

    if (direction == 2)
    {
        angle = angle * (-1);
    }
    // multim5dia_func_set_door_angle(angle);
    // debug_door_ang = angle;

    if (flag)
    {
        hmi_srv.door_status = STAT_IDLE;
    }

    if (1)
    {
        static unsigned long long last_print_time = 0;
        unsigned long long current_time = cm_tick_milli();

        // 只在debug_print_enabled为真时打印
        if (debug_print_enabled && current_time - last_print_time >= 1000)
        {
            LOGD("ypr[0] = %f,door_status = %d , reference_yaw = %f", yaw, hmi_srv.door_status, reference_yaw);
            last_print_time = current_time;
        }
    }

    if (debug_flag)
    {
        LOGD("[door_status] : %d\n", hmi_srv.door_status);
        LOGD("[YAW] : %f", yaw);
    }

    switch (hmi_srv.door_status)
    {
    case STAT_IDLE:
        if (gyroscope_stable(diff_yaw))
        {
            hmi_srv.door_status = STAT_CLOSE;
            log_write(LOG_INFO, TAG, "door status detect [IDLE -> CLOSE]");
            // printf("****door status detect [IDLE -> CLOSE]\n\n");
        }
        break;
    case STAT_CLOSE:
        // diff，diff只是用来过滤掉小的抖动
        door_status = 0;
        ret = door_open_detect(diff_yaw, last_yaw, &reference_yaw, angle);
        if (ret == 1)
        {
            if (hmi_srv.open_cb)
            {
                hmi_srv.open_cb(NULL);
            }
            open_door_time = cm_tick_milli();

            take_photo_on = 1; // 一次开门只拍一张照
            hmi_srv.door_status = STAT_ACTION;
            /*add door open event to list*/
            
            s_door.open_count++;
            if (s_door.open_count >= INT32_MAX)
            {
                s_door.open_count = 1;
            }
            door_open(angle);
            log_write(LOG_INFO, TAG_NAME, "door status detect [CLOSE -> ACTION]\n");
            // printf("****door status detect [CLOSE -> ACTION]\n\n");
        }
        else if (ret == 2)
        {
            // KNJindouGlobal::getInstance().getDoorTimeoutAlarm().stop();
        }
        break;
    case STAT_ACTION:
        if(door_status_flag == 0)
        {
            /*第一次进入开门状态检测*/
            s_door.image_seq = get_image_seq();
            /*由于一次开门拍一次照，因此只需要一位图片序列号*/
            door_status_flag = 1;
        }
        door_status = 1;
        max_angle = max_angle < angle ? angle : max_angle;
        
        if ((angle > min_capture_angle) && (angle < max_capture_angle)) // 拍照
        {
            if(angle > s_door.max_angle)
            {   
                // LOGD("s_door max angel : %f\n",s_door.max_angle);
                s_door.max_angle = angle;
                s_door.angle_current_valid = angle;
                image_cache();
            }
        }

        ret = door_close_detect(angle, diff_yaw, open_door_time);
        if (ret != 0)
        {
            door_close();
            door_take_photo();
            take_photo_on = 0;
            s_door.max_angle = 0;
            hmi_srv.door_status = STAT_CLOSE;
            max_angle = 0;
            /*切换回第一次进入开门标志*/
            door_status_flag = 0;
            log_write(LOG_INFO, TAG_NAME, "door status detect [ACTION -> CLOSE]");
            // printf("****door status detect [ACTION -> CLOSE]\n\n");
        }
        if (ret == 1)
        {
            // KNJindouGlobal::getInstance().getDoorTimeoutAlarm().stop();
        }
        break;
    }

    last_yaw = yaw; // 保存上一次的值
    debug_state = hmi_srv.door_status;
}
//  kill -SIGUSR1 <pid>
static void toggle_debug_print(int signo)
{
    if (signo == SIGUSR1)
    {
        debug_print_enabled = !debug_print_enabled;
        log_write(LOG_INFO, TAG, "Debug print %s", debug_print_enabled ? "enabled" : "disabled");
    }
}

void *hmi_service_thread(void *args)
{
    struct sigaction sa;
    sa.sa_handler = toggle_debug_print;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGUSR1, &sa, NULL) == -1)
    {
        log_write(LOG_ERROR, TAG, "Failed to register signal handler");
    }
    printf("enter hmi_service_thread\n");
    int err_rd_cnt = 0;
    float ypr[3] = {0};
    float last_yaw = 0;
    // uint64_t last_handle = 0;
    char reset_flag = 1;
    unsigned long long lastGsResetTime = cm_tick_milli();
    char gs_open_flag = 0;

    // debug_flag = cm_is_debug();
    while (1)
    {
        if (cm_tick_milli() - lastGsResetTime > 86400000)
        { // 24*60*60*1000
            printf(TAG, "gyroscope cycle reset...");
            lastGsResetTime = cm_tick_milli();
            // gyroscope_close();
            // debug_gs_status = DebugGSStatus::GS_RESET;
            // gs_open_flag = false;
            // sleep(30);
        }

        if (!gs_open_flag)
        {
            if (gyroscope_open() != 0)
            {
                LOGD("gyroscope open fail!");
                // debug_gs_status = DebugGSStatus::GS_OPEN_FAIL;
                gyroscope_close();
                sleep(30);
                continue;
            }
            else
            {
                gs_open_flag = 1;
            }
        }

        if (gyroscope_ready() != 0)
        {
            LOGD("wait gyroscope ready");
            // debug_gs_status = DebugGSStatus::GS_NOT_READY;
            sleep(1);
            continue;
        }
        // usleep()
        if (gyroscope_read_yqr(ypr) == 0)
        {
            // debug_gs_status = DebugGSStatus::GS_READ_SUCC;
            // LOGD("ypr[0] = %f , ypr[1] = %f , ypr[2] = %f",ypr[0], ypr[1] , ypr[2]);
            if (fabs(ypr[0] - last_yaw) < 7.2)
            {
                door_status_detect(ypr[0], reset_flag, 6);
                if (reset_flag)
                {
                    reset_flag = 0;
                }
            }
            else
            {
                LOGD("ypr[0] = %f, last_yaw = %f, fabs(ypr[0] - last_yaw) = %f > 7.2, filtered...", ypr[0], last_yaw, ypr[0] - last_yaw);
            }

            last_yaw = ypr[0];
            err_rd_cnt = 0;
            usleep(20000);
        }
        else
        {
            err_rd_cnt++;
            if (err_rd_cnt > 100)
            {
                LOGD("gyroscope_read fail count > 100");
                reset_flag = 1;

                gyroscope_close();
                // debug_gs_status = DebugGSStatus::GS_READ_FAIL;
                gs_open_flag = 0;
                sleep(30);
            }
        }

#if 0
        if (cm_gettime_milli() - last_handle > 100) // 10Hz
        {
            if (GetDeviceCap()->GetCountTouch() != 0)
            {
                door_handle_detect();
            }
            last_handle = cm_gettime_milli();
        }
#endif
    }

    /*deinit 待修改*/

    // hmi_cmd_dl_deinit();
    // gyroscope_close();
    // hmi_srv_tid = 0;

    return NULL;
}

int door_init()
{
    // s_door.door_config = cm_get_door_config();

    s_door.status = DOOR_CLOSE;
    char cmd[20];
    snprintf(cmd, sizeof(cmd), "mkdir /tmp/data/");
    system(cmd);
    s_door.inited = 1;
    int ret = cm_video_impl_init("t23");
    if (ret != 0)
    {
        LOGD("door_init failed");
        return -1;
    }
    return 0;
}

int door_deinit()
{
    s_door.inited = 0;
    usleep(50000);
    cm_video_impl_deinit();
    return 0;
}



char get_door_status()
{
    return door_status;
}