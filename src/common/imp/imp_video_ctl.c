/*************************************************************************
 * Copyright (C) 2005~2022 Ingenic Semiconductor Co.,Ltd
 *
 * File Name    : imp_video_ctl.c
 * Author       : wxyang
 * Mail         : wxyang@ingenic.com
 * Created Time : 2022-11-23 10:50
 *************************************************************************/
#include <stdio.h>
#include <string.h>
#include <global_config.h>
#include <imp-common.h>
#include <math.h>

/* --------- UVC video process -------- */
#define     AUTO_EXPOSURE_MODE            8
#define     MANUAL_EXPOSURE_MODE          1
#define     UVC_IMG_BYPASS                0

#define     MODULE_TAG                    "UVC_PU/CT"

static IMPISPGamma gamma_cur;
static IMPISPCOEFFTWB isp_core_rgb_coefft_wb_attri_def;

typedef struct _crop_obj {
	int left;
	int top;
	int w;
	int h;
} crop_obj_t;

static crop_obj_t zoom_crop;
static float pan_factor  = 0;
static float tilt_factor = 0;
/* ----------CT process ------------------------------------------------ */

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
	int ret;

	if (!src || !dst) {
		printf("(%s:%d)ERROR: src or dst data is null \n", __func__, __LINE__);
		return -1;
	}

	int value = src[0];

	printf(" auto_exposure_mode set data %d\n", value);
	if(value == AUTO_EXPOSURE_MODE) {
		ret = IMP_ISP_Tuning_SetAeComp(128);
		if(ret)
			printf("ERROR:set exposure time Invalid leave:%d\n",value);

	}
	memcpy(dst, src, len);

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
	int ret;

	if (!src || !dst) {
		printf("(%s:%d)ERROR: src or dst data is null \n", __func__, __LINE__);
		return -1;
	}

	int value_time = 0;
	int value = *((int*) src);

	printf("(%s): value = %d \n", __func__, value);
	switch(value)
	{
	case 1:    value_time = 0; break;
	case 2:    value_time = 25; break;
	case 5:    value_time = 50; break;
	case 10:   value_time = 75; break;
	case 20:   value_time = 100; break;
	case 39:   value_time = 128; break;
	case 78:   value_time = 150; break;
	case 156:  value_time = 175; break;
	case 312:  value_time = 200; break;
	case 625:  value_time = 225; break;
	case 1000: value_time = 255; break;

	default:   value_time = 128;break;

	}

	ret = IMP_ISP_Tuning_SetAeComp(value_time);
	if(ret)
		printf("ERROR:set exposure time Invalid leave:%d\n",value);
	memcpy(dst, src, len);

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
	if (!src || !dst) {
		printf("(%s:%d)ERROR: src or dst data is null \n", __func__, __LINE__);
		return -1;
	}

	/*TODO: other process */
	memcpy(dst, src, len);
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
	if (!src || !dst) {
		printf("(%s:%d)ERROR: src or dst data is null \n", __func__, __LINE__);
		return -1;
	}

	/*TODO: other process */
	memcpy(dst, src, len);
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
	if (!src || !dst) {
		printf("(%s:%d)ERROR: src or dst data is null \n", __func__, __LINE__);
		return -1;
	}

	int ret;
	int zoomwidth_cur,zoomheight_cur;
	float value_cur;

	//int chn_index = video_index < 1 ? UVC_VIDEO_CH : UVC2_VIDEO_CH;

	int value = *((int*) src);
	imp_attr_t *imp_attr = &g_func_param.imp_attr;
	int sensor_width = imp_attr->sensor_info[video_index].sensor_width;
	int sensor_height = imp_attr->sensor_info[video_index].sensor_height;

	value_cur = (float)value;

	zoomwidth_cur = round(sensor_width / sqrt(value_cur / 100));
	zoomwidth_cur = round((float)(zoomwidth_cur / 4)) * 4;
	zoomheight_cur = round(sensor_height / sqrt(value_cur / 100));
	zoomheight_cur = round((float)(zoomheight_cur/ 4)) * 4;

	IMPISPFrontCrop zoom_attr;
	zoom_attr.fcrop_enable = 1;
	zoom_attr.fcrop_left = round((float)(sensor_width - zoomwidth_cur) / 2) * (1 + pan_factor);
	zoom_attr.fcrop_top = round((float)(sensor_height -  zoomheight_cur) / 2) * ( 1 + tilt_factor );
	zoom_attr.fcrop_width = zoomwidth_cur;
	zoom_attr.fcrop_height = zoomheight_cur;

	ret = IMP_ISP_Tuning_SetFrontCrop(&zoom_attr);
	if (ret < 0) {
		printf("IMP Set Fcrop failed=%d\n",__LINE__);
		return -1;
	}

	memcpy(dst, src, len);

	/*save crop_obj */
	zoom_crop.left = zoom_attr.fcrop_left;
	zoom_crop.top = zoom_attr.fcrop_top;
	zoom_crop.w = zoom_attr.fcrop_width;
	zoom_crop.h = zoom_attr.fcrop_height;

	printf("zoom_crpo.left  = %d\n", zoom_crop.left);
	printf("zoom_crpo.top  = %d\n", zoom_crop.top);
	printf("zoom_crpo.w  = %d\n", zoom_crop.w);
	printf("zoom_crpo.h  = %d\n", zoom_crop.h);

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
	int ret = 0;
	uint8_t value[8] = {0};
	int left = 0;
	int top = 0;
	int w = zoom_crop.w;
	int h = zoom_crop.h;
	int pan_set = 0, tilt_set = 0;
	imp_attr_t *imp_attr = &g_func_param.imp_attr;

	//int chn_index = video_index < 1 ? UVC_VIDEO_CH : UVC2_VIDEO_CH;

	if (!src || !dst) {
		printf("(%s:%d)ERROR: src or dst data is null \n", __func__, __LINE__);
		return -1;
	}

	memcpy(value, src, sizeof(uint8_t) * 8);

	/*filter first set caused by enumeration*/
	int pan = value[3] << 24 | value[2] << 16 | value[1] << 8 | value[0];        /* LOW 4 byte */
	int tilt = value[7] << 24 | value[6] << 16 | value[5] << 8 | value [4];      /* Hei 4 byte */
	int sensor_width = imp_attr->sensor_info[video_index].sensor_width;
	int sensor_height = imp_attr->sensor_info[video_index].sensor_height;

	/*changel direction */
	tilt_set = tilt;
	pan_set = -pan;

	pan_factor = 1.0 * pan_set / 36000;
	tilt_factor = 1.0 * tilt_set / 36000;

	left = round(1.0 *(1.0 * (sensor_width - w) / 2) * (1 + 1.0 * pan_set / 36000));
	top = round(1.0 * (1.0 * (sensor_height - h) / 2) * (1 + 1.0 * tilt_set / 36000));

	if (pan_set == 0 && tilt_set == 0) {
		left = zoom_crop.left;
		top = zoom_crop.top;
	}

	IMPISPFrontCrop zoom_attr;
	memset(&zoom_attr, 0, sizeof(IMPISPFrontCrop));
	zoom_attr.fcrop_enable = 1;
	zoom_attr.fcrop_left = left;
	zoom_attr.fcrop_top = top;
	zoom_attr.fcrop_width = w;
	zoom_attr.fcrop_height = h;

#if 0
	ret = IMP_ISP_Tuning_SetAutoZoom(&zoom_attr);
	if (ret < 0) {
		printf("IMP Set Fcrop failed=%d\n",__LINE__);
		return -1;
	}
#else
	ret = IMP_ISP_Tuning_SetFrontCrop(&zoom_attr);
	if (ret < 0) {
		printf("IMP Set Fcrop failed=%d\n",__LINE__);
		return -1;
	}

#endif

	/*
	 *printf("PANTILT_DATA : pan value = %d \n",pan);
	 *printf("PANTILT_DATA : tilt value = %d \n",tilt);
	 *printf("PANTILT left is%d, top is%d,width is %d, height is %d \n",left,top,w,h);
	 */
	memcpy(dst, src, len);
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
	if (!src || !dst) {
		printf("(%s:%d)ERROR: src or dst data is null \n", __func__, __LINE__);
		return -1;
	}

	/*TODO: isp */
	uint8_t value[2] = {0};
	memcpy(value, src, sizeof(uint8_t) * 2);
	int pan = value[1] << 8 | value[0];
	printf("#### roll data = %x####\n", pan);

	int hvflip = pan;

	IMPISPHVFLIP testhv = hvflip;
	IMP_ISP_Tuning_SetHVFLIP(testhv);

	memcpy(dst, src, len);
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
	if (!src || !dst) {
		printf("(%s:%d)ERROR: src or dst data is null \n", __func__, __LINE__);
		return -1;
	}

	/*TODO: isp */

	memcpy(dst, src, len);
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
	if (!src || !dst) {
		printf("(%s:%d)ERROR: src or dst data is null \n", __func__, __LINE__);
		return -1;
	}

	/*IMPVI_NUM sensor = IMPVI_MAIN;*/
	char faceroi[64] = {0};
	int left, top, right, bottom;

	left = ((dst[3] & 0xff) << 8) | (dst[2] & 0xff);
	top = ((dst[1] & 0xff) << 8) | (dst[0] & 0xff);

	right = ((dst[7] & 0xff) << 8) | (dst[6] & 0xff);
	bottom = ((dst[5] & 0xff) << 8) | (dst[4] & 0xff);

	sprintf(faceroi,"video_index: %d, (%d,%d,%d,%d) \n", video_index, left, top, right, bottom);

	/*TODO: isp */

	memcpy(dst, src, len);
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
	int ret;
	if (value < 0 || value > 10) {
		printf("set BackLight Invalid leave:%d\n", value);
		return -1;
	}

	printf("(%s): value is %d \n", __func__, value);
	ret = IMP_ISP_Tuning_SetBacklightComp(value);
	if (ret)
		printf("ERROR: set BackLight Invalid leave:%d\n", value);
	/*backlight_compens.data[UVC_CUR] = value;*/
	/*ucamera_uvc_pu_attr_save(video_index, UVC_BACKLIGHT_COMPENSATION_CONTROL, value);*/
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
	uint32_t value = 0;
	int ret = -1;

	ret = IMP_ISP_Tuning_GetBacklightComp(&value);
	if (ret)
		printf("ERROR: get BackLight error:%d\n", ret);

	printf("(%s): value is %d \n", __func__, value);
	return value;
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
	int ret;

	unsigned char bright = 0;

	bright = value & 0xff;
	ret = IMP_ISP_Tuning_SetBrightness(bright);
	if (ret)
		printf("ERROR: set BrightNess failed :%d\n", ret);
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
	int ret;
	unsigned char bright = 0;

	ret = IMP_ISP_Tuning_GetBrightness(&bright);
	if (ret) {
		printf("get BrightNess failed:%d\n", ret);
		bright = 128;
	}
	return bright;
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
	int ret;
	unsigned char tmp = 0;

	tmp = value & 0xff;
	ret = IMP_ISP_Tuning_SetContrast(tmp);
	if (ret)
		printf("set Contrast failed:%d\n", ret);

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
	int ret;
	unsigned char tmp = 0;

	ret = IMP_ISP_Tuning_GetContrast(&tmp);
	if (ret) {
		printf("get Contrast failed:%d\n", ret);
		tmp = 128;
	}
	return tmp;
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
	int ret;
	int antiflicker_mode;

	antiflicker_mode = value;
	if (antiflicker_mode < IMPISP_ANTIFLICKER_DISABLE || antiflicker_mode >= IMPISP_ANTIFLICKER_BUTT) {
		printf("[ERROR]Sample set PowerLine Freq Invalid level:%d\n", value);
		return 0;
	}

	IMPISPAntiflickerAttr attr;
	attr = (IMPISPAntiflickerAttr)antiflicker_mode;

	ret = IMP_ISP_Tuning_SetAntiFlickerAttr(attr);
	if (ret)
		printf("set PowerLine Freq failed:%d\n", ret);

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
	int ret;
	IMPISPAntiflickerAttr attr;
	int antiflicker_mode = IMPISP_ANTIFLICKER_DISABLE;

	ret = IMP_ISP_Tuning_GetAntiFlickerAttr(&attr);
	if (ret)
		printf("get PowerLine Freq faild:%d\n", ret);

	antiflicker_mode = attr;
	return antiflicker_mode;
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
	int ret;
	unsigned char hue_value = value & 0xff;
	ret = IMP_ISP_Tuning_SetBcshHue(hue_value);
	if (ret)
		printf("set Hue failed:%d\n", ret);
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
	int ret;
	unsigned char hue_value = 0;
	ret = IMP_ISP_Tuning_GetBcshHue(&hue_value);
	if (ret) {
		printf("set Hue failed:%d\n", ret);
		hue_value = 128;
	}
	return hue_value;
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
	int ret;
	unsigned char tmp = 0;
	tmp = value & 0xff;

	ret = IMP_ISP_Tuning_SetSaturation(tmp);
	if (ret)
		printf("set Saturation failed:%d\n", ret);
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
	int ret;
	unsigned char tmp = 0;

	ret = IMP_ISP_Tuning_GetSaturation(&tmp);
	if (ret) {
		printf("get Saturation failed:%d\n", ret);
		tmp = 128;

	}
	return tmp;
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
	int ret;
	unsigned char tmp = 0;

	tmp = value & 0xff;
	ret = IMP_ISP_Tuning_SetSharpness(tmp);
	if (ret)
		printf("set Sharpness failed:%d\n", ret);
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
	int ret;
	unsigned char tmp = 0;

	ret = IMP_ISP_Tuning_GetSharpness(&tmp);
	if (ret) {
		printf("get Sharpness failed:%d\n", ret);
		tmp = 128;
	}
	return tmp;
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
	int i;
	IMPISPGamma gamma;

	memset(&gamma, 0, sizeof(IMPISPGamma));

	/*TODO: set IMPISPGammaCurveType */
	IMP_ISP_Tuning_GetGamma(&gamma_cur);
	memcpy(&gamma, &gamma_cur, sizeof(IMPISPGamma));

	for (i = 0; i < 128; i++) {
		if ((i > 9) && (i < 118))
			continue;
		gamma.gamma[i] = gamma_cur.gamma[i] + 3*(value - 400);
	}

	IMP_ISP_Tuning_SetGamma(&gamma);

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
	int ret;
	unsigned short gain;

	gain = value & 0xffff;

#if 0
	IMPISPWB wb = {0};
	if (mode == ISP_CORE_WB_MODE_AUTO)
		wb.mode = mode;
	else{
		wb.mode = ISP_CORE_WB_MODE_MANUAL;
		wb.bgain = gain;
		wb.rgain = 160000/gain;
	}
	ret = IMP_ISP_Tuning_SetWB(&wb);
	if (ret)
		printf("set WhiteBalance failed:%d\n", ret);
#endif
	IMPISPCOEFFTWB isp_core_rgb_coefft_wb_attr;
	ret = IMP_ISP_Tuning_Awb_GetRgbCoefft(&isp_core_rgb_coefft_wb_attr);
	//printf("--- the r = %d, the g = %d, the b =%d --- \n",isp_core_rgb_coefft_wb_attr.rgb_coefft_wb_r,isp_core_rgb_coefft_wb_attr.rgb_coefft_wb_g,isp_core_rgb_coefft_wb_attr.rgb_coefft_wb_b);
	if (ret)
		printf("get rgbcoefft failed:%d\n", ret);

	//printf("+++ value = %d +++\n",value);
	if (gain > 4500) {
		isp_core_rgb_coefft_wb_attr.rgb_coefft_wb_r = (gain-4500)/26 +1024;
		isp_core_rgb_coefft_wb_attr.rgb_coefft_wb_g = 1024 - (gain - 4500)/500;
		isp_core_rgb_coefft_wb_attr.rgb_coefft_wb_b = 1024 - (gain -4500)/9;
	} else {
		isp_core_rgb_coefft_wb_attr.rgb_coefft_wb_r = 1024 - (4500-gain)/4;
		isp_core_rgb_coefft_wb_attr.rgb_coefft_wb_g = 1024;
		isp_core_rgb_coefft_wb_attr.rgb_coefft_wb_b = (4500-gain)/7 + 1024;
	}
	//printf("####the wb cur mode = %d, rgain = %d bgain = %d######\n",attr.mode, attr.gain_val.rgain, attr.gain_val.bgain);
	ret = IMP_ISP_Tuning_Awb_SetRgbCoefft(&isp_core_rgb_coefft_wb_attr);
	if (ret)
		printf("set WhiteBalance failed:%d\n", ret);
	//printf("the r = %d, the g = %d, the b =%d \n",isp_core_rgb_coefft_wb_attr.rgb_coefft_wb_r,isp_core_rgb_coefft_wb_attr.rgb_coefft_wb_g,isp_core_rgb_coefft_wb_attr.rgb_coefft_wb_b);

	return 0;
}

/*
 * @func: uvc_pu_wbt_auto_set
 * @brief: PU_WHITE_BLANCE_TEMPERATURE_AUTO_CONTROL.
 * @param:
 * 	video_index: uvc channel

 * @return:
 * 	-1 error, 0 succeed
 */
int uvc_pu_wbt_auto_set(int video_index)
{

	int ret;

	ret = IMP_ISP_Tuning_Awb_SetRgbCoefft(&isp_core_rgb_coefft_wb_attri_def);
	if (ret)
		printf("set WhiteBalance failed:%d\n", ret);

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
	int ret, tmp = 0;
	static int first_set = 0;
#if 0
	IMPISPWB wb = {0};

	ret = IMP_ISP_Tuning_GetWB(&wb);
	if (ret)
		printf("get WhiteBalance failed:%d\n", ret);
	if (wb.mode == ISP_CORE_WB_MODE_AUTO)
		wb.bgain = 417;
	tmp = (wb.mode << 16) | wb.bgain;
	return tmp;
#endif

	IMPISPCOEFFTWB isp_core_rgb_coefft_wb_attr;
	ret = IMP_ISP_Tuning_Awb_GetRgbCoefft(&isp_core_rgb_coefft_wb_attr);
	if (ret)
		printf("get rgbcoefft failed:%d\n", ret);

	if (!first_set) {
		ret = IMP_ISP_Tuning_Awb_GetRgbCoefft(&isp_core_rgb_coefft_wb_attri_def);
		//printf("--- the r = %d, the g = %d, the b =%d --- \n",isp_core_rgb_coefft_wb_attr.rgb_coefft_wb_r,isp_core_rgb_coefft_wb_attr.rgb_coefft_wb_g,isp_core_rgb_coefft_wb_attr.rgb_coefft_wb_b);
		if (ret)
			printf("get rgbcoefft failed:%d\n", ret);

		isp_core_rgb_coefft_wb_attri_def.rgb_coefft_wb_r = 1024;
		isp_core_rgb_coefft_wb_attri_def.rgb_coefft_wb_g = 1024;
		isp_core_rgb_coefft_wb_attri_def.rgb_coefft_wb_b = 1024;
		first_set++;
	}

	/*IMPISPWBAttr attr;*/
	/*ret = IMP_ISP_Tuning_GetAwbAttr(sensor, &attr);*/
	/*if (ret)*/
	/*printf("get WhiteBalance failed:%d\n", ret);*/

	/*if (attr.mode == ISP_CORE_WB_MODE_AUTO)*/
	/*attr.gain_val.bgain = 128;*/
	/*tmp = (attr.mode << 16) | attr.gain_val.bgain;*/

	return tmp;
}
