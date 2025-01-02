#include "stdint.h"
// #include "cm_mpu_ctrl.h"
#include <imp/imp_log.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include "cm_config.h"
#include "cm_video_ctrl.h"
#include "cm_utils.h"
#include "cm_conf.h"
#include "cm_config.h"
#include <signal.h>
#include <unistd.h> // for access()
#include "cm_common.h"
#include "wind_process_manager.h"
#include "wind_connect_up.h"
#include "yq_mpu.h"
#include "mpu_ctrl.h"
#include "circular_log.h"
static char debug_flag = 0;
static char *TAG = "door detect";
static float debug_reference_ang = 0.0;
static volatile int debug_print_enabled = 0; // 0: 不打印，1: 打印
/****************config param ******************/
#define DEFAULE_FAB_STABLE_DIFF 0.01f
#define DEFAULT_DOOR_DIRECTION 1
#define DEFAULT_CAPUURE_ANGLE 4.0f
#define DEFAULT_FILTER_ANGLE 4.0f
#define DEFAULT_FAB_IDLE_DIFF 0.01f
static float fab_stable_diff;
static int door_direction;
static float angle_open;
static float angle_close;
static float angle_capture;
static float fab_idle_diff;
// extern float Filter_Angle;
static int calib_time;
#define CM_CONFIG_FILE "/system/etc/cm_config.ini"
static CMConfig cm_config_local[100];
static char *TAG_NAME = "door detect";
#define DEBUG_FLAG_FILE "/tmp/door_debug_enable"
// 检查调试标志文件是否存在
static int check_debug_flag_file(void)
{
    return (access(DEBUG_FLAG_FILE, F_OK) == 0);
}
/****************config param ******************/
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
    float angle_last_valid;

    AngleTolerance angle_tolerant;
    DoorOpenTimeout door_open_timeout;

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

int cm_config_get_stable_diff(float *angle)
{
    char *v = cm_config_value(cm_config_local, 20, "door_stable_diff");
    if (v != NULL)
    {
        *angle = atof(v);
    }
    else
    {
        *angle = DEFAULE_FAB_STABLE_DIFF;
    }
    return 0;
}

int cm_config_get_door_direction(int *angle)
{
    char *v = cm_config_value(cm_config_local, 20, "door_direction");
    if (v != NULL)
    {
        *angle = atoi(v);
    }
    else
    {
        *angle = DEFAULT_DOOR_DIRECTION;
    }
    return 0;
}
int cm_config_get_calib_time(int *angle)
{
    char *v = cm_config_value(cm_config_local, 20, "calib_time");
    if (v != NULL)
    {
        *angle = atoi(v);
    }
    else
    {
        *angle = 10;
    }
    return 0;
}

int cm_config_get_door_angle_capture(float *angle)
{
    char *v = cm_config_value(cm_config_local, 20, "angle_capture");
    if (v != NULL)
    {
        *angle = atof(v);
    }
    else
    {
        *angle = DEFAULT_CAPUURE_ANGLE;
    }
    return 0;
}
int cm_config_get_door_Filter_Angle(float *angle)
{
    char *v = cm_config_value(cm_config_local, 20, "Filter_Angle");
    if (v != NULL)
    {
        *angle = atof(v);
    }
    else
    {
        *angle = DEFAULT_FILTER_ANGLE;
    }
    return 0;
}

int cm_config_get_door_idle_diff(float *angle)
{
    char *v = cm_config_value(cm_config_local, 40, "fab_idle_diff");
    if (v != NULL)
    {
        *angle = atof(v);
    }
    else
    {
        *angle = DEFAULT_FAB_IDLE_DIFF;
    }
    return 0;
}
static int get_config()
{
    int ret = cm_config_read(CM_CONFIG_FILE, cm_config_local, 50);
    if (ret > 0)
    {
        for (int i = 0; i < ret; i++)
        {
            LOGD("key [%s] value [%s]", cm_config_local[i].key, cm_config_local[i].value);
        }
        cm_config_get_stable_diff(&fab_stable_diff);
        cm_config_get_door_direction(&door_direction);
        cm_config_get_angle_open(&angle_open);
        cm_config_get_angle_close(&angle_close);
        cm_config_get_door_angle_capture(&angle_capture);
        // fab_idle_diff = cm_config_local[ret-1].value;
        cm_config_get_door_idle_diff(&fab_idle_diff);
        // cm_config_get_door_Filter_Angle(&Filter_Angle);
        cm_config_get_calib_time(&calib_time);
        LOGD("cm_config_load [fab_stable_diff] %f\n", fab_stable_diff);
        LOGD("cm_config_load [door_direction] %d\n", door_direction);
        LOGD("cm_config_load [angle_open] %f\n", angle_open);
        LOGD("cm_config_load [angle_close] %f\n", angle_close);
        LOGD("cm_config_load [angle_capture] %f\n", angle_capture);
        LOGD("cm_config_load [fab_idle_diff] %f\n", fab_idle_diff);
        // LOGD("cm_config_load [Filter_Angle] %f\n", Filter_Angle);
        LOGD("cm_config_load [calib_time] %d\n", calib_time);
    }
    else
    {
        LOGD("Error: cm_config_load failed. Use default");
    }
    return 0;
}

static int image_save(DoorInfoItem *item)
{
    // DoorInfoItem *last_item = cm_door_info_item_get(item->array_index);
    // if (last_item)
    // {
    //     // 删除一个
    //     char remove_path[256] = {0};
    //     snprintf(remove_path, sizeof(remove_path), "/tmp/data/%llu",
    //              last_item->open_count);

    //     char cmd_remove_path[256] = {0};
    //     snprintf(cmd_remove_path, sizeof(cmd_remove_path), "rm -rf %s", remove_path);
    //     LOGD("remove [%s]\n", cmd_remove_path);
    //     system(cmd_remove_path);
    // }

    // 创建新的
    char new_path[256] = {0};
    snprintf(new_path, sizeof(new_path), "mkdir /tmp/data/%llu", item->open_count);
    LOGD("mkdir new [%s]\n", new_path);
    system(new_path);

    snprintf(item->image_path, sizeof(item->image_path), "/tmp/data/%llu/img-%llu-%d.jpg", item->open_count,
             item->open_count, (int)item->image_angle);

    // 保存文件内容
    if (s_door.buf.length > 0)
    {
        IMP_LOG_DBG(TAG, "image save to file %s of len [%d]\n", item->image_path, s_door.buf.length);
        int fd = open(item->image_path, O_WRONLY | O_CREAT);
        if (fd > 0)
        {
            write(fd, s_door.buf.start, s_door.buf.length);
            close(fd);
        }
    }
    // 更新信息信息
    cm_door_info_item_add(item);
    return 0;
}

static int door_close()
{
    DoorInfoItem item;
    item.open_count = s_door.open_count;
    item.image_angle = s_door.angle_last_valid;
    item.array_index = item.open_count % 5;
    LOGD("door close open count [%llu] angle now [%f] array_index [%d]\n",
         item.open_count, item.image_angle, item.array_index);
    wind_process_manager_status_changed(PROCESS_STATUS_IDLE);
    wind_connect_up_door_close(&item);
    return 0;
}
static int door_close_not_save_img()
{
    DoorInfoItem item;
    item.open_count = s_door.open_count;
    item.image_angle = s_door.angle_last_valid;
    item.array_index = item.open_count % 5;
    LOGD("door close open count [%llu] angle now [%f] array_index [%d]\n",
         item.open_count, item.image_angle, item.array_index);
    wind_process_manager_status_changed(PROCESS_STATUS_IDLE);
    wind_connect_up_door_close(&item);
    return 0;
}
static int door_open(float angle)
{
    wind_process_manager_status_changed(PROCESS_STATUS_DOOR_OPEN);
    wind_connect_up_door_open(s_door.open_count, angle);
    return 0;
}

static int image_cache()
{
    DoorInfoItem item;
    item.open_count = s_door.open_count;
    item.image_angle = s_door.angle_last_valid;
    item.array_index = item.open_count % 5;
    char new_path[256] = {0};
    snprintf(new_path, sizeof(new_path), "mkdir /tmp/data/%llu", item.open_count);
    LOGD("mkdir new [%s]\n", new_path);
    system(new_path);

    snprintf(item.image_path, sizeof(item.image_path), "/tmp/data/img-%llu-%d.jpg",
             item.open_count, (int)item.image_angle);

    cm_video_take_photo_save_to_file(item.image_path);
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
    static uint32_t dyn_cnt = 0, stable_cnt = 0, stable_flag = 0;
    static door_act_e action = DOOR_IDLE;

    if (debug_flag)
    {
        printf("diff [%f] last [%f] reference [%f] ang [%f] stable_cnt [%d] dyn_cnt [%d] stable_flag [%d] action [%d]\n",
               diff, last_ang, *reference_yaw, ang, stable_cnt, dyn_cnt, stable_flag, action);
    }
    switch (action)
    {
    case DOOR_IDLE:
        if (fabs(diff) < fab_idle_diff)
        {
            stable_cnt++;
            dyn_cnt = 0;

            if (stable_cnt > 10)
            {                              // 稳定约150ms
                *reference_yaw = last_ang; // 记录下门动作的初始角度值，应用层上消除陀螺仪零漂
                stable_cnt = 0;
                stable_flag = 1;
            }
        }
        else
        {
            dyn_cnt++;
            stable_cnt = 0;
            if (dyn_cnt == 3)
            {
                dyn_cnt = 0;
                if (stable_flag)
                { // 抖动前稳定过，防止缓慢开门小角度重复触发
                    LOGD("door detect [IDLE -> SHAKE]\n");
                    action = DOOR_SHAKE;
                    stable_flag = 0;
                }
            }
        }
        break;
    case DOOR_SHAKE:
        dyn_cnt++;
        if (ang > 4.0)
        { // 开门角度阈值
            action = DOOR_OPEN;
            LOGD("door detect [SHAKE -> OPEN]\n");
            dyn_cnt = 0;
        }

        if (dyn_cnt > 200 || ang < -2.0)
        { // 约1s无开门动作，则判定为抖动；角度为负则为关门动作
            action = DOOR_IDLE;
            LOGD("door detect [SHAKE -> IDLE]\n");
            dyn_cnt = 0;
            return 2;
        }
        break;
    case DOOR_OPEN:
        action = DOOR_IDLE;
        LOGD("door detect [OPEN -> IDLE]\n");
        break;
    }

    debug_action = action;
    debug_reference_ang = *reference_yaw;
    return (action == DOOR_OPEN) ? 1 : 0;
}

static uint8_t door_close_detect(float ang, float diff, float *reference_yaw, long openTime)
{
    static uint16_t cnt = 0;
    static uint16_t cnt2 = 0;
    if (debug_flag)
    {
        printf("cnt [%d] cnt2 [%d] ang [%f] diff [%f] reference_yaw [%f]\n", cnt, cnt2, ang, diff, *reference_yaw);
    }

    // 判断关门情况一:开门角度小于4度，且实时角度偏差小于0.1度，持续100毫秒
    if ((ang < angle_close) && (fabs(diff) < 0.1))
    {
        cnt++;
    }
    else
    {
        cnt = 0;
    }

    if (cnt >= 10)
    { // 静止约100ms
        cnt = 0;
        return 1;
    }

    // 判断关门情况二: 长时间未发现关门动作（可能在上货、陀螺仪数据异常）*/
    if (cm_tick_milli() - openTime > 5 * 60 * 1000)
    {
        IMP_LOG_DBG(TAG, "Door long time no close");
        return 2;
    }
    return 0;
}

static uint8_t gyroscope_stable(float diff)
{
    static uint8_t stable_cnt = 0;
    if (fabs(diff) < fab_stable_diff)
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
    // static uint8_t open_cnt = 0;
    // static uint8_t close_cnt = 0;
    int ret = 0;
    static long open_door_time = 0;

    diff_yaw = yaw - last_yaw;
    angle = yaw - reference_yaw; // 实时角度
    if (angle <= -180)           // 零漂180度附近开门
    {
        angle = 360 + angle;
    }
    if (angle >= 180 && angle <= 360) // 零漂180度边界处理
    {
        angle = 360 - angle;
    }

    /*获取门的开方向*/
    int direction = door_direction;
    if (direction == 2)
    {
        angle = angle * (-1);
    }
    // multimedia_func_set_door_angle(angle);
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
            log_write(LOG_INFO, TAG, "ypr[0] = %f,door_status = %d , reference_yaw = %f", yaw, hmi_srv.door_status, reference_yaw);
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
            door_open(angle);
            s_door.open_count++;
            if (s_door.open_count >= INT32_MAX)
            {
                s_door.open_count = 1;
            }
            log_write(LOG_INFO, TAG_NAME, "door status detect [CLOSE -> ACTION]\n");
            // printf("****door status detect [CLOSE -> ACTION]\n\n");
        }
        else if (ret == 2)
        {
            // KNJindouGlobal::getInstance().getDoorTimeoutAlarm().stop();
        }
        break;
    case STAT_ACTION:
        max_angle = max_angle < angle ? angle : max_angle;

        if (take_photo_on && angle > capture_angle) // 拍照
        {
            s_door.angle_last_valid = max_angle;
            // LOGD("Image Cache\n");
            log_write(LOG_INFO, TAG_NAME, "Image Cache ,angle [%f]", angle);
            image_cache();
            take_photo_on = 0;
        }

        ret = door_close_detect(angle, diff_yaw, &reference_yaw, open_door_time);
        if (ret != 0)
        {
            door_close();
            take_photo_on = 0;
            hmi_srv.door_status = STAT_CLOSE;
            max_angle = 0;
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

static void toggle_debug_print(int signo)
{
    if (signo == SIGUSR1)
    {
        debug_print_enabled = !debug_print_enabled;
        log_write(LOG_INFO, TAG, "Debug print %s", debug_print_enabled ? "enabled" : "disabled");
    }
}
// extern int debug1;
int cm_is_debug2();
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
    get_config();
    int err_rd_cnt = 0;
    float ypr[3] = {0};
    float last_yaw = 0;
    // uint64_t last_handle = 0;
    char reset_flag = 1;
    unsigned long long lastGsResetTime = cm_tick_milli();
    char gs_open_flag = 0;

    debug_flag = cm_is_debug();
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

            if (fabs(ypr[1] - last_yaw) < 7.2)
            {
                door_status_detect(ypr[1], reset_flag, angle_capture);
                if (reset_flag)
                {
                    reset_flag = 0;
                }
            }
            else
            {
                LOGD("ypr[0] = %f, last_yaw = %f, fabs(ypr[0] - last_yaw) = %f > 7.2, filtered...", ypr[0], last_yaw, ypr[0] - last_yaw);
            }

            last_yaw = ypr[1];
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
    s_door.door_config = cm_get_door_config();

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
    // if (s_door.video_impl)
    // {
    //     s_door.video_impl->stop(&s_door.ctx);
    //     s_door.video_impl->close(&s_door.ctx);
    // }
    cm_video_impl_deinit();
    return 0;
}
