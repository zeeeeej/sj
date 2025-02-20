#define LOG_TAG "[DOOE_DETECT]"
#include "stdint.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <unistd.h>
#include <signal.h> // 为了使用 sig_atomic_t
#include <mqueue.h>

#include "cm_video_ctrl.h"
#include "cm_utils.h"
#include "cm_conf.h"
#include "cm_common.h"
#include "yq_mpu.h"
#include "mpu_ctrl.h"
#include "circular_log.h"
#include "ImageInfoList.h"
#include "Cam485ProtocolCommon.h"
#include "AttributeTable.h"
#include "cm_config.h"
// 当次开门缓存下来的信息
typedef struct
{
    float image_angle;
    char image_path[256];
    unsigned long long open_count;
    int array_index;
} DoorInfoItem;

typedef struct
{
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

enum
{
    DOOR_CLOSE = 0,
    DOOR_OPENED,
};

typedef struct
{
    int status;
    int inited;
    DoorConfigArgs *door_config;
    // 当前角度
    float angle_now;
    // 开门次数
    unsigned long long open_count;
    // 开门后的最大角度
    float max_angle;
    float angle_last_valid;
    float angle_current_valid;
    uint8_t image_seq;
    pthread_t door_detect_pid;
    mqd_t door_detect_msg_queue;
    volatile sig_atomic_t stop_thread; // 标志位用于通知线程退出
} DoorDetect;
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

static char debug_flag = 0;

static float debug_reference_ang = 0.0;
static volatile int debug_print_enabled = 1; // 0: 不打印，1: 打印

/*动态拍照*/
static float min_capture_angle = 55.0f;
static float max_capture_angle = 65.0f;
static char door_status = 0;

/*配置信息*/
static uint8_t gyroscope_enable_status = 0;
static uint8_t gyroscope_capture_direction = 0;
static uint8_t gyroscope_capture_angle = 0;     /*静态拍照*/

/*默认配置*/
#define GYROSCOPE_ENABLE_STATUS 1
#define GYROSCOPE_CAPTURE_DIRECTION 0   /*0:逆时针，1:顺时针*/
#define GYROSCOPE_CAPTURE_ANGLE 60      /*开门角度*/

/*编译选项*/
#define DYNAMIC_CAPTURE_ENABLE 0     /*0:静态拍照，1:动态拍照*/


static DoorDetect s_door;
static door_act_e debug_action = DOOR_IDLE;
static int debug_state = STAT_IDLE;
static hmi_srv_t hmi_srv;

#define CM_CONFIG_FILE "/system/etc/cm_config.ini"

#define DOOR_ERR_SUCCESS     0
#define DOOR_ERR_INITIALIZED -1
#define DOOR_ERR_NOT_INIT    -2
#define DOOR_ERR_VIDEO_INIT  -3
#define DOOR_ERR_THREAD      -4
#define DOOR_ERR_NOT_ENABLE  -5
static int door_close()
{

    return 0;
}

static int door_take_photo()
{

    return 0;
}

static int door_open(float angle)
{

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
    system(cmd_remove_path);
    deleteNodeByFileName(old_path);
    DoorInfoItem item;
    item.open_count = s_door.open_count;
    item.image_angle = s_door.angle_current_valid;
    item.array_index = item.open_count % 5;
    snprintf(item.image_path, sizeof(item.image_path), "/tmp/gyro_trigger/image_%d_%d.jpg",
             s_door.image_seq, (int)item.image_angle);
    ret = cm_video_take_photo_save_to_file(item.image_path);
    if (ret != 0)
    {
        log_e("image cache fail");
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
    static uint32_t dyn_cnt = 0, stable_cnt = 0, stable_flag = 0;
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
            { // 稳定约200ms
                dyn_cnt = 0;
                *reference_yaw = last_ang; // 记录下门动作的初始角度值，应用层上消除陀螺仪零漂
                stable_cnt = 0;
                stable_flag = 1;
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
                    action = DOOR_SHAKE;
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

    debug_action = action;
    debug_reference_ang = *reference_yaw;
    return (action == DOOR_OPEN) ? 1 : 0;
}

static uint8_t door_close_detect(float ang, float diff, long openTime)
{
    static uint16_t cnt = 0;
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
    { // 静止约200ms
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
    int ret = 0;
    static long open_door_time = 0;
    /*表示是否第一次进入开门状态检测*/
    static uint8_t door_status_flag = 0;

    diff_yaw = yaw - last_yaw;
    angle = yaw - reference_yaw; // 实时角度

    if (gyroscope_capture_direction == 1)
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
            log_i("door status detect [IDLE -> CLOSE]");
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
            hmi_srv.door_status = STAT_ACTION;

            s_door.open_count++;
            if (s_door.open_count >= INT32_MAX)
            {
                s_door.open_count = 1;
            }
            door_open(angle);
            log_i("door status detect [CLOSE -> ACTION]");
        }
        else if (ret == 2)
        {
        }
        break;
    case STAT_ACTION:

        door_status = 1;
        max_angle = max_angle < angle ? angle : max_angle;
#if DYNAMIC_CAPTURE_ENABLE
/*动态拍照:在最低阈值和最高阈值取一张*/
        if ((angle > min_capture_angle) && (angle < max_capture_angle)) // 拍照
        {
            if (door_status_flag == 0)
            {
                /*进入开门状态检测,并且到达指定拍照角度,获取图片序列号*/
                s_door.image_seq = get_image_seq();
                door_status_flag = 1;
            }
            if (angle > s_door.max_angle)
            {
                s_door.max_angle = angle;
                s_door.angle_current_valid = angle;
                image_cache();
            }
        }
#else
/*静态拍照：大于角度阈值只拍一张图*/
        if (angle > gyroscope_capture_angle)
        {
            if(take_photo_on == 0)
            {
                s_door.image_seq = get_image_seq();
                take_photo_on = 1;
                // s_door.max_angle = angle;
                s_door.angle_current_valid = angle;
                image_cache();
            }
            
        }
#endif
        ret = door_close_detect(angle, diff_yaw, open_door_time);
        if (ret != 0)
        {
            door_close();
            door_take_photo();
            s_door.max_angle = 0;
            hmi_srv.door_status = STAT_CLOSE;
            max_angle = 0;
            take_photo_on = 0;
            /*切换回第一次进入开门标志*/
            door_status_flag = 0;
            log_i("door status detect [ACTION -> CLOSE]");
        }
        if (ret == 1)
        {
        }
        break;
    }

    last_yaw = yaw; // 保存上一次的值
    debug_state = hmi_srv.door_status;
}

void *door_detect_thread(void *args)
{

    log_i("Enter door detect thread");
    int err_rd_cnt = 0;
    float ypr[3] = {0};
    float last_yaw = 0;
    char reset_flag = 1;
    unsigned long long lastGsResetTime = cm_tick_milli();
    char gs_open_flag = 0;
    while (!(s_door.stop_thread))
    {
        if (cm_tick_milli() - lastGsResetTime > 86400000)
        { // 24*60*60*1000
            log_i("gyroscope cycle reset...");
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
                log_e("gyroscope open fail!");
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
            sleep(1);
            continue;
        }
        if (gyroscope_read_ypr(ypr) == 0)
        {
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
    }
    gyroscope_close();
    log_i("exit door detect thread");
    return NULL;
}

/*获取陀螺仪配置*/
static int get_door_config()
{
    int ret = 0 ; 
    ret = Get_Gyroscope_Enable_Status(&gyroscope_enable_status);
    if(ret != 0)
    {
        log_w("get gyroscope enable status fail , use default value : %d",GYROSCOPE_ENABLE_STATUS);
        /*获取陀螺仪开启状态失败，使用默认值*/
        gyroscope_enable_status = GYROSCOPE_ENABLE_STATUS;/*设置为开启*/
    }
    ret = Get_Gyroscope_Capture_image_direction(&gyroscope_capture_direction);
    if(ret != 0)
    {
        log_w("get gyroscope capture image direction fail , use default value : %d",GYROSCOPE_CAPTURE_DIRECTION);
        gyroscope_capture_direction = GYROSCOPE_CAPTURE_DIRECTION;/*设置为默认逆时针抓图*/
    }
    uint8_t angle_valid = 0;
    uint8_t angle_a = 0;
    uint8_t angle_b = 0;    
    ret = Get_Gyroscope_Capture_image_angle(&angle_a, &angle_b);
    if(ret != 0)
    {
        log_w("get gyroscope capture image angle fail , use default value : %d",GYROSCOPE_CAPTURE_ANGLE);
        gyroscope_capture_angle = GYROSCOPE_CAPTURE_ANGLE;/*设置为默认开门角度*/
    }
    else
    {
        if(angle_a!=0)
        {

            gyroscope_capture_angle = angle_a;
        }
        else if(angle_b!=0)
        {
            gyroscope_capture_angle = angle_b;
        }
        else
        {
            /*都为0则使用默认值*/
            gyroscope_capture_angle = GYROSCOPE_CAPTURE_ANGLE;
        }
        log_i("get gyroscope capture image angle success , angle : %d",gyroscope_capture_angle);
    }

    return ret;
}


int door_detect_init()
{
    int ret = 0 ;
    // 检查是否已经初始化
    if (s_door.inited) {
        log_w("door detect already initialized");
        return DOOR_ERR_INITIALIZED;
    }
    ret = get_door_config();
    if(ret != 0)
    {
        log_e("get door config fail");
        /*获取失败不关闭陀螺仪线程*/
    }
    else
    {
        if(gyroscope_enable_status == 1)
        {
            log_i("gyroscope enable");
        }
        else
        {
            log_i("gyroscope disable");
            /*读取配置文件，如果是关闭陀螺仪，则直接退出*/
            return DOOR_ERR_NOT_ENABLE;
        }
    }
    // 初始化结构体
    memset(&s_door, 0, sizeof(s_door));  // 清空结构体
    s_door.status = DOOR_CLOSE;
    s_door.stop_thread = 0;
    
    // 创建数据目录
    char cmd[20];
    snprintf(cmd, sizeof(cmd), "mkdir -p /tmp/data/");
    if (system(cmd) != 0) {
        log_e("Failed to create data directory");
        return DOOR_ERR_NOT_INIT;
    }
    
    // 初始化视频模块
    ret = cm_video_impl_init("t23");
    if (ret != 0) {
        log_e("door_init failed: video init error");
        return DOOR_ERR_VIDEO_INIT;
    }

    // 创建检测线程
    ret = pthread_create(&s_door.door_detect_pid, NULL, door_detect_thread, NULL);
    if (ret != 0) {
        log_e("Error creating thread: %s", strerror(ret));
        cm_video_impl_deinit();
        return DOOR_ERR_THREAD;
    }
    
    // 设置初始化标志
    s_door.inited = 1;
    log_i("door detect initialized successfully");
    return DOOR_ERR_SUCCESS;
}

int door_detect_deinit()
{
    // 检查是否已经初始化
    if (!s_door.inited) {
        log_w("door detect not initialized or already deinitialized");
        return DOOR_ERR_NOT_INIT;
    }

    // 设置停止标志并等待线程退出
    s_door.stop_thread = 1;
    if (pthread_join(s_door.door_detect_pid, NULL) != 0) {
        log_e("Error joining thread");
        // 继续清理，但返回错误码
        return DOOR_ERR_THREAD;
    }

    // 清理资源
    /*不关闭视频模块*/
    // cm_video_impl_deinit();
    // 清空结构体
    memset(&s_door, 0, sizeof(s_door));
    
    log_i("door detect deinitialized successfully");
    return DOOR_ERR_SUCCESS;
}

char get_door_status()
{
    return door_status;
}


int set_gyroscope_capture_direction(uint8_t capture_direction)
{
    gyroscope_capture_direction = capture_direction;
    return 0;
}

int set_gyroscope_capture_angle(uint8_t capture_angle)
{
    gyroscope_capture_angle = capture_angle;
    return 0;
}