/*********************************************************
 * File Name   : sample_uvc_video_ctl.c
 * Author      : Tania Xiang
 * Mail        : xiuhui.xiang@ingenic.com
 * Created Time: 2023-02-24 17:34
 ********************************************************/

#include <stdio.h>
#include <string.h>
#include <global_config.h>
#include <imp-common.h>
#include <math.h>


/*
 * @func: uvc_ct_ae_mode_set
 * @brief: CT_AE_MODE_CONTROL
 * @param:
 * 	video_index: uvc channel
 * 	src:         host set value
 * 	dst:         device save value
 * 	len:         data length

 * @return:
 * 	-1 error, 0 succeed
 */
int uvc_ct_ae_mode_set(int video_index, char* src, char* dst, int len)
{

	return 0;
}

/*
 * @func: uvc_ct_exposure_time_abs_set
 * @brief: CT_EXPOSURE_TIME_ABSOLUTE_CONTROL
 * @param:
 * 	video_index: uvc channel
 * 	src:         host set value
 * 	dst:         device save value
 * 	len:         data length

 * @return:
 * 	-1 error, 0 succeed
 */
int uvc_ct_exposure_time_abs_set(int video_index, char* src , char *dst, int len)
{

	return 0;
}

/*
 * @func: uvc_ct_focus_abs_set
 * @brief: CT_FOCUS_ABSOLUTE_CONTROL
 * @param:
 * 	video_index: uvc channel
 * 	src:         host set value
 * 	dst:         device save value
 * 	len:         data length

 * @return:
 * 	-1 error, 0 succeed
 */
int uvc_ct_focus_abs_set(int video_index, char* src, char* dst,  int len)
{
	return 0;
}

/*
 * @func: uvc_ct_focus_auto_set
 * @brief: CT_FOCUS_AUTO_CONTROL
 * @param:
 * 	video_index: uvc channel
 * 	src:         host set value
 * 	dst:         device save value
 * 	len:         data length

 * @return:
 * 	-1 error, 0 succeed
 */
int uvc_ct_focus_auto_set(int video_index, char* src, char* dst,  int len)
{
	return 0;
}

/*
 * @func: uvc_ct_zoom_abs_set
 * @brief: CT_ZOOM_ABSOLUTE_CONTROL
 * @param:
 * 	video_index: uvc channel
 * 	src:         host set value
 * 	dst:         device save value
 * 	len:         data length

 * @return:
 * 	-1 error, 0 succeed
 */
int uvc_ct_zoom_abs_set(int video_index, char* src, char* dst,  int len)
{
	return 0;
}

/*
 * @func: uvc_ct_pantilt_abs_set
 * @brief: CT_PANTILT_ABSOLUTE_CONTROL
 * @param:
 * 	video_index: uvc channel
 * 	src:         host set value
 * 	dst:         device save value
 * 	len:         data length

 * @return:
 * 	-1 error, 0 succeed
 */
int uvc_ct_pantilt_abs_set(int video_index, char* src, char* dst,  int len)
{
	return 0;
}


/*
 * @func: uvc_ct_roll_abs_set
 * @brief: CT_ROLL_ABSOLUTE_CONTROL
 * @param:
 * 	video_index: uvc channel
 * 	src:         host set value
 * 	dst:         device save value
 * 	len:         data length

 * @return:
 * 	-1 error, 0 succeed
 */
int uvc_ct_roll_abs_set(int video_index, char* src, char* dst,  int len)
{
	return 0;
}

/*
 * @func: uvc_ct_privacy_set
 * @brief: CT_PRIVACY_CONTROL
 * @param:
 * 	video_index: uvc channel
 * 	src:         host set value
 * 	dst:         device save value
 * 	len:         data length

 * @return:
 * 	-1 error, 0 succeed
 */
int uvc_ct_privacy_set(int video_index, char* src, char* dst,  int len)
{
	return 0;
}

/*
 * @func: uvc_ct_roi_set
 * @brief: CT_REGION_OF_INTERST_CONTROL
 * @param:
 * 	video_index: uvc channel
 * 	src:         host set value
 * 	dst:         device save value
 * 	len:         data length

 * @return:
 * 	-1 error, 0 succeed
 */
int uvc_ct_roi_set(int video_index, char* src, char* dst,  int len)
{

	return 0;
}
/* ---------- END of CT process ------------------------------------------------ */

/* ---------- PU process ------------------------------------------------ */

/*
 * @func: uvc_pu_backlight_compens_set
 * @brief: PU_BACKLIGHT_COMPENSATION_CONTROL
 * @param:
 * 	video_index: uvc channel
 * 	value :         host set value

 * @return:
 * 	-1 error, 0 succeed
 */
int uvc_pu_backlight_compens_set(int video_index, int value)
{

	return 0;
}

/*
 * @func: uvc_pu_backlight_compens_get
 * @brief: PU_BACKLIGHT_COMPENSATION_CONTROL
 * @param:
 * 	video_index: uvc channel

 * @return:
 * 	-1 error, 0 succeed
 */
int uvc_pu_backlight_compens_get(int video_index)
{

	return 0;
}

/*
 * @func: uvc_pu_brightness_set
 * @brief: PU_BACKLIGHT_COMPENSATION_CONTROL
 * @param:
 * 	video_index: uvc channel
 * 	value:         host set value

 * @return:
 * 	-1 error, 0 succeed
 */
int uvc_pu_brightness_set(int video_index, int value)
{

	return 0;
}

/*
 * @func: uvc_pu_brightness_get
 * @brief: PU_BACKLIGHT_COMPENSATION_CONTROL
 * @param:
 * 	video_index: uvc channel

 * @return:
 * 	-1 error, 0 succeed
 */
int uvc_pu_brightness_get(int video_index)
{
	return 0;
}

/*
 * @func: uvc_pu_contrast_set
 * @brief: PU_CONSTRAST_CONTROL
 * @param:
 * 	video_index: uvc channel
 * 	value:         host set value

 * @return:
 * 	-1 error, 0 succeed
 */
int uvc_pu_contrast_set(int video_index, int value)
{

	return 0;
}

/*
 * @func: uvc_pu_contrast_get
 * @brief: PU_CONSTRAST_CONTROL
 * @param:
 * 	video_index: uvc channel

 * @return:
 * 	-1 error, 0 succeed
 */
int uvc_pu_contrast_get(int video_index)
{

	return 0;
}

/*
 * @func: uvc_pu_contrast_auto_set
 * @brief: PU_CONSTRAST_AUTO_CONTROL
 * @param:
 * 	video_index: uvc channel
 * 	value:         host set value

 * @return:
 * 	-1 error, 0 succeed
 */
int uvc_pu_contrast_auto_set(int video_index, int value)
{
	/* TODO: */
	return 0;
}

/*
 * @func: uvc_pu_contrast_auto_get
 * @brief: PU_CONSTRAST_AUTO_CONTROL
 * @param:
 * 	video_index: uvc channel

 * @return:
 * 	-1 error, 0 succeed
 */
int uvc_pu_contrast_auto_get(int video_index)
{
	/* TODO: */
	return 0;
}

/*
 * @func: uvc_pu_gain_set
 * @brief: PU_GAIN_CONTROL
 * @param:
 * 	video_index: uvc channel
 * 	value:         host set value

 * @return:
 * 	-1 error, 0 succeed
 */
int uvc_pu_gain_set(int video_index, int value)
{
	/* TODO: */
	return 0;
}

/*
 * @func: uvc_pu_gain_get
 * @brief: PU_GAIN_CONTROL
 * @param:
 * 	video_index: uvc channel

 * @return:
 * 	-1 error, 0 succeed
 */
int uvc_pu_gain_get(int video_index)
{
	/* TODO: */
	return 0;
}

/*
 * @func: uvc_pu_powerlinefreq_set
 * @brief: PU_POWER_LINE_FREQUENCY_CONTROL
 * @param:
 * 	video_index: uvc channel
 * 	value:         host set value

 * @return:
 * 	-1 error, 0 succeed
 */
int uvc_pu_powerlinefreq_set(int video_index, int value)
{
	return 0;
}

/*
 * @func: uvc_pu_powerlinefreq_get
 * @brief: PU_POWER_LINE_FREQUENCY_CONTROL
 * @param:
 * 	video_index: uvc channel

 * @return:
 * 	-1 error, 0 succeed
 */
int uvc_pu_powerlinefreq_get(int video_index)
{

	return 0;
}

/*
 * @func: uvc_pu_hue_set
 * @brief: PU_HUE_CONTROL
 * @param:
 * 	video_index: uvc channel
 * 	value:         host set value

 * @return:
 * 	-1 error, 0 succeed
 */
int uvc_pu_hue_set(int video_index, int value)
{

	return 0;
}

/*
 * @func: uvc_pu_hue_get
 * @brief: PU_HUE_CONTROL
 * @param:
 * 	video_index: uvc channel

 * @return:
 * 	-1 error, 0 succeed
 */
int uvc_pu_hue_get(int video_index)
{

	return 0;
}

/*
 * @func: uvc_pu_hue_auto_set
 * @brief: PU_HUE_AUTO_CONTROL. not support
 * @param:
 * 	video_index: uvc channel
 * 	value:         host set value

 * @return:
 * 	-1 error, 0 succeed
 */
int uvc_pu_hue_auto_set(int video_index, int value)
{
	/*TODO: */
	return 0;
}

/*
 * @func: uvc_pu_hue_auto_get
 * @brief: PU_HUE_AUTO_CONTROL. not support
 * @param:
 * 	video_index: uvc channel

 * @return:
 * 	-1 error, 0 succeed
 */
int uvc_pu_hue_auto_get(int video_index, int value)
{
	/*TODO: */
	return 0;
}

/*
 * @func: uvc_pu_saturation_set
 * @brief: PU_HUE_AUTO_CONTROL.
 * @param:
 * 	video_index: uvc channel
 * 	value:         host set value

 * @return:
 * 	-1 error, 0 succeed
 */
int uvc_pu_saturation_set(int video_index, int value)
{

	return 0;
}

/*
 * @func: uvc_pu_saturation_get
 * @brief: PU_HUE_AUTO_CONTROL.
 * @param:
 * 	video_index: uvc channel

 * @return:
 * 	-1 error, 0 succeed
 */
int uvc_pu_saturation_get(int video_index)
{

	return 0;
}

/*
 * @func: uvc_pu_sharpness_set
 * @brief: PU_SHARPNESS_CONTROL.
 * @param:
 * 	video_index: uvc channel
 * 	value:         host set value

 * @return:
 * 	-1 error, 0 succeed
 */
int uvc_pu_sharpness_set(int video_index, int value)
{

	return 0;
}

/*
 * @func: uvc_pu_sharpness_get
 * @brief: PU_SHARPNESS_CONTROL.
 * @param:
 * 	video_index: uvc channel

 * @return:
 * 	-1 error, 0 succeed
 */
int uvc_pu_sharpness_get(int video_index)
{

	return 0;
}

/*
 * @func: uvc_pu_gamma_set
 * @brief: PU_SHARPNESS_CONTROL.
 * @param:
 * 	video_index: uvc channel
 * 	value:         host set value

 * @return:
 * 	-1 error, 0 succeed
 */
int uvc_pu_gamma_set(int video_index, int value)
{

	return 0;
}

/*
 * @func: uvc_pu_wbt_set
 * @brief: PU_WHITE_BLANCE_TEMPERATURE_CONTROL.
 * @param:
 * 	video_index: uvc channel
 * 	value:         host set value

 * @return:
 * 	-1 error, 0 succeed
 */
int uvc_pu_wbt_set(int video_index, int value)
{

	return 0;
}

/*
 * @func: uvc_pu_wbt_get
 * @brief: PU_WHITE_BLANCE_TEMPERATURE_CONTROL.
 * @param:
 * 	video_index: uvc channel
 * 	value:         host set value

 * @return:
 * 	-1 error, 0 succeed
 */
int uvc_pu_wbt_get(int video_index)
{
	return 0;
}

int imp_sdk_init(int video_index, int format, int width, int height)
{
	return 0;
}
int imp_sdk_deinit(int video_index, int format)
{
	return 0;
}
int sample_get_jpeg_snap(int chn_num, char *img_buf)
{
	FILE *fp = NULL;
	int length;

	fp = fopen("/system/bin/1920x1080.jpeg", "r");
	if(!fp) {
		printf("read error!\n");
		return -1;
	}

	fseek(fp, 0L, SEEK_END);
	length = ftell(fp);

	fseek(fp, 0L, SEEK_SET);
	fread(img_buf, 1, length, fp);

	fclose(fp);

	return length;
}

int sample_get_h264_snap(int chn_num, char *img_buf)
{
	return 0;
}

int sample_get_yuv_snap(int chn_num, char *img_buf)
{
	return 0;
}
