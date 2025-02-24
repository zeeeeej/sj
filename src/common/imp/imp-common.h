// /*
//  * sample-common.h
//  *
//  * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
//  */

// #ifndef __SAMPLE_COMMON_H__
// #define __SAMPLE_COMMON_H__

// #include <imp/imp_log.h>
// #include <imp/imp_system.h>
// #include <imp/imp_common.h>
// #include <imp/imp_osd.h>
// #include <imp/imp_framesource.h>
// #include <imp/imp_isp.h>
// #include <imp/imp_encoder.h>
// #include <imp/imp_audio.h>
// #include <unistd.h>

// #ifdef __cplusplus
// #if __cplusplus
// extern "C"
// {
// #endif
// #endif /* __cplusplus */

// /* no such MACRO in kernel, so defined here */
// #define v4l2_fourcc(a, b, c, d)\
// ((__u32)(a) | ((__u32)(b) << 8) | ((__u32)(c) << 16) | ((__u32)(d) << 24))
// #define V4L2_PIX_FMT_H265     v4l2_fourcc('H', '2', '6', '5')

// #define UCAMERA_DEBUG                 0
// #if UCAMERA_DEBUG
//   #define Ucamera_DEBUG(format, arg...) \
//       printf("%s: " format "\n" , "[Ucamera]", ## arg)
// #else
//   #define Ucamera_DEBUG(format, arg...)
// #endif

// #define UVC_VIDEO_CH                  0
// #define UVC2_VIDEO_CH                 1
// #define ALGORITHM_VIDEO_CH            1

// /************* Sensor info *************/
// #define SENSOR_FRAME_RATE_NUM_25      25
// #define SENSOR_FRAME_RATE_NUM_30      30
// #define SENSOR_FRAME_RATE_DEN         1

// #define SENSOR_MAIN                   0
// #define SENSOR_NUM                    1
// #define SENSOR_CUBS_TYPE              TX_SENSOR_CONTROL_INTERFACE_I2C

// #define FIRST_SENSOR_NAME             "gc2053" //sensor name (match with snesor driver name)
// #define FIRST_I2C_ADDR                0x37     //sensor i2c address
// #define FIRST_SENSOR_WIDTH            1920     //sensor width
// #define FIRST_SENSOR_HEIGHT           1080     //sensor height
// #define FIRST_DEFAULT_BOOT            1        //sensor default mode(0/1/2/3/4)

// #define SENSOR_WIDTH_THIRD            1920
// #define SENSOR_HEIGHT_THIRD           1080

// #define SENSOR_WIDTH_SECOND         640
// #define SENSOR_HEIGHT_SECOND        360

// /************** OSD info ***************/
// #define OSD_REGION_WIDTH              16
// #define OSD_REGION_HEIGHT             34

// /*********** Framsource info ***********/
// #define FS_CHN_NUM                    2 //MIN 1,MAX 3

// #define CH0_INDEX                     0
// #define CH1_INDEX                     1
// #define CH2_INDEX                     2

// #define CHN0_EN                       1
// #define CHN1_EN                       1
// #define CHN2_EN                       0
// #define CROP_EN                       0

// #define CHN_NUM ARRAY_SIZE(chn)

// /*********** Encoder info ***********/
// #define ENCODER_MJPEG_CHANNEL_INDEX_PLUS        3

// /*********** Struct define ***********/
// struct chn_conf {
//   unsigned int index;//0 for main channel ,1 for second channel
//   unsigned int enable;
//   IMPPayloadType payloadType;
//   IMPFSChnAttr fs_chn_attr;
//   IMPCell framesource_chn;
//   IMPCell imp_encoder;
// };

// /*********** Function declaration ***********/
// int sample_system_init(void *param);
// int sample_system_exit();

// int imp_sdk_init(int video_index, int format, int width, int height);
// int imp_sdk_deinit(int video_index, int format);

// int sample_get_jpeg_snap(int chn_num, char *img_buf);
// int sample_get_h264_snap(int chn_num, char *img_buf);
// int sample_get_yuv_snap(int chn_num, char *img_buf);

// int sample_get_ucamera_streamonoff();
// int sample_get_ucamera_af_onoff();
// int sample_get_ucamera_af_cur();

// void sample_audio_amic_init(void);
// void sample_audio_amic_exit();
// int sample_audio_amic_pcm_get(short *pcm);

// int sample_speak_init(void);
// void sample_speak_exit(void);

// /*
//  * @func: uvc_fu_mic_mute_set
//  * @brief: MUTE_CONTROL
//  * @param:
//  *   mute: mute value

//  * @return:
//  *   -1 error, 0 succeed
//  */
// int uac_fu_mic_mute_set(int mute);

// /*
//  * @func: uac_fu_mic_volume_set
//  * @brief: VOLMUE_CONTROL
//  * @param:
//  *   db: volume

//  * @return:
//  *   -1 error, 0 succeed
//  */
// int uac_fu_mic_volume_set(int16_t db);

// /*
//  * @func: uac_fu_spk_mute_set
//  * @brief: MUTE_CONTROL
//  * @param:
//  *   mute: mute value

//  * @return:
//  *   -1 error, 0 succeed
//  */
// int uac_fu_spk_mute_set(int mute);

// /*
//  * @func: uac_fu_spk_volume_set
//  * @brief: VOLMUE_CONTROL
//  * @param:
//  *   db: volume

//  * @return:
//  *   -1 error, 0 succeed
//  */
// int uac_fu_spk_volume_set(int16_t db);

// /*uvc callback  */
// /*
//  * @func: uvc_ct_ae_mode_set
//  * @brief: CT_AE_MODE_CONTROL
//  * @param:
//  *   video_index: uvc channel
//  *   src:         host set value
//  *   dst:         device save value
//  *   len:         data length

//  * @return:
//  *   -1 error, 0 succeed
//  */
// int uvc_ct_ae_mode_set(int video_index, char* src, char* dst, int len);

// /*
//  * @func: uvc_ct_exposure_time_abs_set
//  * @brief: CT_EXPOSURE_TIME_ABSOLUTE_CONTROL
//  * @param:
//  *   video_index: uvc channel
//  *   src:         host set value
//  *   dst:         device save value
//  *   len:         data length

//  * @return:
//  *   -1 error, 0 succeed
//  */
// int uvc_ct_exposure_time_abs_set(int video_index, char* src , char *dst, int len);

// /*
//  * @func: uvc_ct_focus_abs_set
//  * @brief: CT_FOCUS_ABSOLUTE_CONTROL
//  * @param:
//  *   video_index: uvc channel
//  *   src:         host set value
//  *   dst:         device save value
//  *   len:         data length

//  * @return:
//  *   -1 error, 0 succeed
//  */
// int uvc_ct_focus_abs_set(int video_index, char* src, char* dst,  int len);

// /*
//  * @func: uvc_ct_focus_auto_set
//  * @brief: CT_FOCUS_AUTO_CONTROL
//  * @param:
//  *   video_index: uvc channel
//  *   src:         host set value
//  *   dst:         device save value
//  *   len:         data length

//  * @return:
//  *   -1 error, 0 succeed
//  */
// int uvc_ct_focus_auto_set(int video_index, char* src, char* dst,  int len);

// /*
//  * @func: uvc_ct_zoom_abs_set
//  * @brief: CT_ZOOM_ABSOLUTE_CONTROL
//  * @param:
//  *   video_index: uvc channel
//  *   src:         host set value
//  *   dst:         device save value
//  *   len:         data length

//  * @return:
//  *   -1 error, 0 succeed
//  */
// int uvc_ct_zoom_abs_set(int video_index, char* src, char* dst,  int len);

// /*
//  * @func: uvc_ct_pantilt_abs_set
//  * @brief: CT_PANTILT_ABSOLUTE_CONTROL
//  * @param:
//  *   video_index: uvc channel
//  *   src:         host set value
//  *   dst:         device save value
//  *   len:         data length

//  * @return:
//  *   -1 error, 0 succeed
//  */
// int uvc_ct_pantilt_abs_set(int video_index, char* src, char* dst,  int len);

// /*
//  * @func: uvc_ct_roll_abs_set
//  * @brief: CT_ROLL_ABSOLUTE_CONTROL
//  * @param:
//  *   video_index: uvc channel
//  *   src:         host set value
//  *   dst:         device save value
//  *   len:         data length

//  * @return:
//  *   -1 error, 0 succeed
//  */
// int uvc_ct_roll_abs_set(int video_index, char* src, char* dst,  int len);

// /*
//  * @func: uvc_ct_privacy_set
//  * @brief: CT_PRIVACY_CONTROL
//  * @param:
//  *   video_index: uvc channel
//  *   src:         host set value
//  *   dst:         device save value
//  *   len:         data length

//  * @return:
//  *   -1 error, 0 succeed
//  */
// int uvc_ct_privacy_set(int video_index, char* src, char* dst,  int len);

// /*
//  * @func: uvc_ct_roi_set
//  * @brief: CT_REGION_OF_INTERST_CONTROL
//  * @param:
//  *   video_index: uvc channel
//  *   src:         host set value
//  *   dst:         device save value
//  *   len:         data length

//  * @return:
//  *   -1 error, 0 succeed
//  */
// int uvc_ct_roi_set(int video_index, char* src, char* dst,  int len);

// /*
//  * @func: uvc_pu_backlight_compens_set
//  * @brief: PU_BACKLIGHT_COMPENSATION_CONTROL
//  * @param:
//  *   video_index: uvc channel
//  *   value :         host set value

//  * @return:
//  *   -1 error, 0 succeed
//  */
// int uvc_pu_backlight_compens_set(int video_index, int value);

// /*
//  * @func: uvc_pu_backlight_compens_get
//  * @brief: PU_BACKLIGHT_COMPENSATION_CONTROL
//  * @param:
//  *   video_index: uvc channel

//  * @return:
//  *   -1 error, 0 succeed
//  */
// int uvc_pu_backlight_compens_get(int video_index);

// /*
//  * @func: uvc_pu_brightness_set
//  * @brief: PU_BACKLIGHT_COMPENSATION_CONTROL
//  * @param:
//  *   video_index: uvc channel
//  *   value:         host set value

//  * @return:
//  *   -1 error, 0 succeed
//  */
// int uvc_pu_brightness_set(int video_index, int value);

// /*
//  * @func: uvc_pu_brightness_get
//  * @brief: PU_BACKLIGHT_COMPENSATION_CONTROL
//  * @param:
//  *   video_index: uvc channel

//  * @return:
//  *   -1 error, 0 succeed
//  */
// int uvc_pu_brightness_get(int video_index);

// /*
//  * @func: uvc_pu_contrast_set
//  * @brief: PU_CONSTRAST_CONTROL
//  * @param:
//  *   video_index: uvc channel
//  *   value:         host set value

//  * @return:
//  *   -1 error, 0 succeed
//  */
// int uvc_pu_contrast_set(int video_index, int value);

// /*
//  * @func: uvc_pu_contrast_get
//  * @brief: PU_CONSTRAST_CONTROL
//  * @param:
//  *   video_index: uvc channel

//  * @return:
//  *   -1 error, 0 succeed
//  */
// int uvc_pu_contrast_get(int video_index);

// /*
//  * @func: uvc_pu_contrast_auto_set
//  * @brief: PU_CONSTRAST_AUTO_CONTROL. not support
//  * @param:
//  *   video_index: uvc channel
//  *   value:         host set value

//  * @return:
//  *   -1 error, 0 succeed
//  */
// int uvc_pu_contrast_auto_set(int video_index, int value);

// /*
//  * @func: uvc_pu_contrast_auto_get. not support
//  * @brief: PU_CONSTRAST_AUTO_CONTROL
//  * @param:
//  *   video_index: uvc channel

//  * @return:
//  *   -1 error, 0 succeed
//  */
// int uvc_pu_contrast_auto_get(int video_index);

// /*
//  * @func: uvc_pu_gain_set
//  * @brief: PU_GAIN_CONTROL. not support
//  * @param:
//  *   video_index: uvc channel
//  *   value:         host set value

//  * @return:
//  *   -1 error, 0 succeed
//  */
// int uvc_pu_gain_set(int video_index, int value);

// /*
//  * @func: uvc_pu_gain_get
//  * @brief: PU_GAIN_CONTROL
//  * @param:
//  *   video_index: uvc channel

//  * @return:
//  *   -1 error, 0 succeed
//  */
// int uvc_pu_gain_get(int video_index);

// /*
//  * @func: uvc_pu_powerlinefreq_set
//  * @brief: PU_POWER_LINE_FREQUENCY_CONTROL
//  * @param:
//  *   video_index: uvc channel
//  *   value:         host set value

//  * @return:
//  *   -1 error, 0 succeed
//  */
// int uvc_pu_powerlinefreq_set(int video_index, int value);

// /*
//  * @func: uvc_pu_powerlinefreq_get
//  * @brief: PU_POWER_LINE_FREQUENCY_CONTROL
//  * @param:
//  *   video_index: uvc channel

//  * @return:
//  *   -1 error, 0 succeed
//  */
// int uvc_pu_powerlinefreq_get(int video_index);

// /*
//  * @func: uvc_pu_hue_set
//  * @brief: PU_HUE_CONTROL
//  * @param:
//  *   video_index: uvc channel
//  *   value:         host set value

//  * @return:
//  *   -1 error, 0 succeed
//  */
// int uvc_pu_hue_set(int video_index, int value);

// /*
//  * @func: uvc_pu_hue_get
//  * @brief: PU_HUE_CONTROL
//  * @param:
//  *   video_index: uvc channel

//  * @return:
//  *   -1 error, 0 succeed
//  */
// int uvc_pu_hue_get(int video_index);

// /*
//  * @func: uvc_pu_hue_auto_set
//  * @brief: PU_HUE_AUTO_CONTROL. not support
//  * @param:
//  *   video_index: uvc channel
//  *   value:         host set value

//  * @return:
//  *   -1 error, 0 succeed
//  */
// int uvc_pu_hue_auto_set(int video_index, int value);

// /*
//  * @func: uvc_pu_hue_auto_get
//  * @brief: PU_HUE_AUTO_CONTROL. not support
//  * @param:
//  *   video_index: uvc channel

//  * @return:
//  *   -1 error, 0 succeed
//  */
// int uvc_pu_hue_auto_get(int video_index, int value);

// /*
//  * @func: uvc_pu_saturation_set
//  * @brief: PU_HUE_AUTO_CONTROL.
//  * @param:
//  *   video_index: uvc channel
//  *   value:         host set value

//  * @return:
//  *   -1 error, 0 succeed
//  */
// int uvc_pu_saturation_set(int video_index, int value);

// /*
//  * @func: uvc_pu_saturation_get
//  * @brief: PU_HUE_AUTO_CONTROL.
//  * @param:
//  *   video_index: uvc channel

//  * @return:
//  *   -1 error, 0 succeed
//  */
// int uvc_pu_saturation_get(int video_index);

// /*
//  * @func: uvc_pu_sharpness_set
//  * @brief: PU_SHARPNESS_CONTROL.
//  * @param:
//  *   video_index: uvc channel
//  *   value:         host set value

//  * @return:
//  *   -1 error, 0 succeed
//  */
// int uvc_pu_sharpness_set(int video_index, int value);

// /*
//  * @func: uvc_pu_sharpness_get
//  * @brief: PU_SHARPNESS_CONTROL.
//  * @param:
//  *   video_index: uvc channel

//  * @return:
//  *   -1 error, 0 succeed
//  */
// int uvc_pu_sharpness_get(int video_index);

// /*
//  * @func: uvc_pu_gamma_set
//  * @brief: PU_GAMMA_CONTROL.
//  * @param:
//  *   video_index: uvc channel
//  *   value:         host set value

//  * @return:
//  *   -1 error, 0 succeed
//  */
// int uvc_pu_gamma_set(int video_index, int value);

// /*
//  * @func: uvc_pu_wbt_set
//  * @brief: PU_WHITE_BLANCE_TEMPERATURE_CONTROL.
//  * @param:
//  *   video_index: uvc channel
//  *   value:         host set value

//  * @return:
//  *   -1 error, 0 succeed
//  */
// int uvc_pu_wbt_set(int video_index, int value);

// /*
//  * @func: uvc_pu_wbt_get
//  * @brief: PU_WHITE_BLANCE_TEMPERATURE_CONTROL.
//  * @param:
//  *   video_index: uvc channel

//  * @return:
//  *   -1 error, 0 succeed
//  */
// int uvc_pu_wbt_get(int video_index);

// /*
//  * @func: uvc_pu_wbt_auto_set
//  * @brief: PU_WHITE_BLANCE_TEMPERATURE_AUTO_CONTROL.
//  * @param:
//  *   video_index: uvc channel

//  * @return:
//  *   -1 error, 0 succeed
//  */
// int uvc_pu_wbt_auto_set(int video_index);

// int sample_sensor_setfps(int sensor, int fps_num, int fps_den);

// #ifdef __cplusplus
// #if __cplusplus
// }
// #endif
// #endif /* __cplusplus */

// #endif /* __SAMPLE_COMMON_H__ */
