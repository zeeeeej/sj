#ifndef __GLOBAL_CONFIG__
#define __GLOBAL_CONFIG__

#include <stdint.h>
#include <imp-common.h>
#include <usbcamera.h>

#define UCAMERA_DEBUG           0

#if UCAMERA_DEBUG
#define Ucamera_DEBUG(format, arg...)                                            \
	printf("%s: " format "\n" , "[Ucamera]", ## arg)
#else
#define Ucamera_DEBUG(format, arg...)
#endif

#ifndef KIVA_VERSION
#define KIVA_VERSION                            "V0.0.0"
#endif

#define UVC_CONFIG_FILE_PATH			"/system/uvc.config"
#define UVC_ATTR_FILE_PATH			"/system/uvc.attr"
#define UVC2_ATTR_FILE_PATH			"/system/uvc2.attr"
#define MERT_MODEL_FILE_PATH			"/system/etc/model/AEC_T41_16K_SS_UC.mgk"

#define VENDOR_ID           0xa108	// ingenic VID
#define PRODUCT_ID          0x2240	// 2022.T40
#define DEVICE_BCD          0x0105
#define MANUFACT_LABEL      "Ingenic Semiconductor Co.,Ltd"
#define PRODUCT_LABEL       "Ingenic HD Web Camera"
#define SERIAL_LABEL        "Ucamera001"

#define H264_MAX_WIDTH      3840
#define H264_MAX_HEIGHT     2160
#define H264_MAX_FPS        25

#define UVC_FORMAT_MAX		5

#define AMIC_ENABLE             	   1
#define DMIC_ENABLE     	           1
#define SPK_ENABLE	                   1
#define AEC_ENABLE			   1
#define MIC_VOL                            0
#define SPK_VOL                            0
#define MIC_CHNCNT                         1
#define SAMPLERATE_DEF			16000
#define BITWIDTH_DEF			16
#define SOUNDMODE_DEF			1

#define ADB_ENABLE                         1
#define STILLCAP_ENABLE                    1
#define H264_ENABLE                        1
#define H265_ENABLE                        1

#define AF_ENABLE                              1
#define AF_DISABLE                             0
#define FOCUS_TRIGGER_VALUE                    15
#define MOTOR_PIXEL                            400
#define MOTOR_STEP                             25
#define MOTOR_STEP_MAX                         600
#define MOTOR_STEP_MIN                         300
#define MAX_STEP                               20
#define MIN_STEP                               10
#define MOTOR_STEP_TIME                        (80 * 1000)

#ifdef MODULE_FACEZOOM_ENABLE
#define FACEZOOM_ENABLE 					   1
#define SINGLE_MODE							   1
#define MULTI_MODE							   2
#endif

#ifdef MODULE_PIP_ENABLE
#define PIP_ENABLE 					   1
#endif

#ifdef OSD_ENABLE
#define DISABLE_OSD				0
#define IPU_OSD					1
#define ISP_OSD					2
#define IPU_ISP_OSD				3
#define OSD_MEM                                 1
#define OSD_EN                                  1
#endif

/*AF Param */
typedef struct _af_param {
	uint8_t af_en;                          /* AF 使能 */
	uint8_t focus_trigger_value;            /* AF 触发值 */
	uint16_t motor_pixel;                   /* 权重范围 */
	uint16_t motor_step;                    /* 电机步长   用于老代码传统af，暂时保留*/
	uint16_t motor_step_max;                /* vcm 行程最大值 */
	uint16_t motor_step_min;                /* vcm 行程最小值 */
	uint16_t max_step;                      /* 电机最大步长 */
	uint16_t min_step;                      /* 电机最小步长 */
	uint32_t motor_sleep_time;              /* 电机移动等待时间 */
} af_param_t;

typedef struct _sensor_info {
	uint8_t default_boot;
	uint8_t sensor_fps;
	uint16_t i2c_addr;                     /* i2c_addr */
	uint16_t sensor_width;                 /* sensor width */
	uint16_t sensor_height;                /* sensor height */
	uint8_t sensor_name[LABEL_LEN];        /* sesnor_name */
} sensor_info_t;

typedef struct _encoder_info {
	uint8_t rcmode;                        /* 编码模式 */
	uint8_t gop;
	uint8_t qp_value;                      /* qp value */
	uint32_t bitrate;                      /* 码率 */
#ifdef T41
	uint8_t qp_reform;                     /* mjpeg qp 动态变化 */
#endif

} encoder_info_t;

typedef struct {
	unsigned char bcsh_en;
	unsigned char bcsh_low;
	unsigned char bcsh_high;
} bcsh_info_t;

#if defined(T41) || defined(T23)
typedef struct _ivdc_info{
	uint8_t ivdc_mode;
	uint8_t ivdc_jpegv;
} ivdc_info_t;
#endif

/* IMP ISP Attr */
typedef struct _imp_attr {
	uint8_t hvflip;
	uint8_t dynamic_fps;
#ifdef OSD_ENABLE
	uint8_t osd_mode;                    /* 0: disable osd 1: ipu osd, 2: isp osd, 3: ipu osd and isp osd */
	uint8_t osd_mem;                     /* 1：512K  2：512*2K */
#endif
	uint8_t wdr_en;

#if defined(T41) || defined (T23)
	ivdc_info_t ivdc_info;
#endif
	bcsh_info_t bcsh_info;
	sensor_info_t sensor_info[VIDEO_NUM];
	encoder_info_t encoder_info;
} imp_attr_t;

/* motor_track param */
//#define MOTOR_TRACK_ENABLE                          0
#define MOTOR_TRACK_DIRECTION                       1
#define MOTOR_TRACK_HVFLIP                          1
#define MOTOR_TRACK_LIMIT                           0
#define MOTOR_TRACK_RESET_SPEED                     750
#define MOTOR_TRACK_LENS_HOV                        88
#define MOTOR_TRACK_HMAXSTEP                        4096
#define MOTOR_TRACK_UCAMERA_ENABLE                  1
#define MOTOR_TRACK_RUN_SPEED_PERCENT               1.0
typedef struct _motor_track_attr {
	uint8_t motor_track_en;                     /* 电机是否打开 */
	uint8_t ucamera_en;                         /* ucamera_en */
	uint8_t direction;                          /* 移动方向 */
	uint8_t islimit;                            /* 是否限位 */
	uint16_t reset_speed;                       /* 初始速度 */
	uint16_t lens_hov;                          /* 水平方向视场角 */
	uint8_t hvflip;                             /* 镜像 */
	uint32_t hmaxstep;                          /* 电机转动一圈需要的最大脉冲数 */
	float run_speed_percent;                    /* 电机运行速度百分比 */
} motor_track_attr_t;

#ifdef MODULE_FACEZOOM_ENABLE
typedef struct _facezoom_func_param_t {
	uint8_t facezoom_en;
	uint8_t facezoom_mode;
} facezoom_func_param_t;
#endif

#ifdef MODULE_PIP_ENABLE
typedef struct _pip_func_param_t {
	uint8_t pip_en;
	uint8_t pip_mode;
} pip_func_param_t;
#endif

typedef struct _config_func_param {
	device_info_t dev_info;			/* usb 设备信息 */
	audio_info_t audio_info;		/* uac 配置 */
	video_info_t video_info[VIDEO_NUM];	/* uvc 配置 */

	imp_attr_t imp_attr;			/* imp 可配参数配置 */
	af_param_t af_param;			/* auto focus 参数配置 */
	motor_track_attr_t motor_track_param;	/* 云台功能配置 */
#ifdef MODULE_FACEZOOM_ENABLE
	facezoom_func_param_t facezoom_param;	/* C位功能配置 */
#endif
#ifdef MODULE_PIP_ENABLE
	pip_func_param_t pip_param;		/* pip功能配置 */
#endif

} config_func_param_t;

extern config_func_param_t g_func_param;

/* config init function */
int system_config_init();
void system_config_deinit();

/* other process thread */
int kiva_process_init();
void kiva_process_deinit();

#endif
