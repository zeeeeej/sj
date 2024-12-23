/*********************************************************
 * File Name   : src/modules/module_ucamera/module_ucamera_control.c
 * Author      : Tania Xiang
 * Mail        : xiuhui.xiang@ingenic.com
 * Created Time: 2022-05-30 20:04
 ********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "uvc_control.h"
#include "uac_control.h"

#define MODULE_TAG "module_ucamera"

/*=============================
 * ucamera system init
 *============================*/
int ucamera_system_init(config_func_param_t *cfg_param)
{
	int ret;

	/* Step first */
	ret = IF_Ucam_Init();
	if (ret < 0) {
		printf("ERROR(%s): IF_Ucam_Init failed!\n", MODULE_TAG);
		return -1;
	}

	/* Step second */
	ret = IF_Ucam_Device_Config(cfg_param->dev_info);
	if (ret < 0) {
		printf("ERROR(%s): IF_Ucam_Device_Config failed!\n", MODULE_TAG);
		return -1;
	}



	/* Set uvc bulk transfer */
	ret = IF_Ucam_Bulk_Mode();
	if (ret < 0) {
		printf("ERROR(%s): IF_Ucam_Bulk_Mode failed!\n", MODULE_TAG);
		return -1;
	}

#if 0
	/* Set uvc fps default */
	ret = IF_Ucam_Video_Fps_Default(20, 0);
	if (ret < 0) {
		printf("ERROR(%s): IF_Ucam_Video_Fps_Default failed!\n", MODULE_TAG);
		return -1;
	}

	/* Set uvc bulk transfer */
	ret = IF_Ucam_Bulk_Mode();
	if (ret < 0) {
		printf("ERROR(%s): IF_Ucam_Bulk_Mode failed!\n", MODULE_TAG);
		return -1;
	}

	/* Set uvc interrupt reduction level */
	ret = IF_Ucam_UVC_Interrupt_level(4);
	if (ret < 0) {
		printf("ERROR(%s): IF_Ucam_UVC_Interrupt_level failed!\n", MODULE_TAG);
		return -1;
	}

	/* Set full speed support */
	ret = IF_Ucam_FS_Enable();
	if (ret < 0) {
		printf("ERROR(%s): IF_Ucam_FS_Enable failed!\n", MODULE_TAG);
		return -1;
	}

	/* Set v4l2 info buff size */
	v4l2_info_t v4l2_info_test;
	v4l2_info_test.compress_factor = 0.25;
	ret = IF_Ucam_V4l2_Set_Config(&v4l2_info_test);
	if (ret < 0) {
		printf("ERROR(%s): IF_UCAM_V4l2_Set_Config failed!\n", MODULE_TAG);
		return -1;
	}
#endif

	/* Steps ... */
	ret = IF_Ucam_Video_Config(0, cfg_param->video_info[0]);
	if (ret < 0) {
		printf("ERROR(%s): IF_Ucam_Video_Config(0) failed!\n", MODULE_TAG);
		return -1;
	}
#if (VIDEO_NUM == 2)
	ret = IF_Ucam_Video_Config(1, cfg_param->video_info[1]);
	if (ret < 0) {
		printf("ERROR(%s): IF_Ucam_Video_Config(1) failed!\n", MODULE_TAG);
		return -1;
	}
#endif

#ifdef MODULE_UAC
	ret = IF_Ucam_Audio_Config(cfg_param->audio_info);
	if (ret < 0) {
		printf("ERROR(%s): IF_Ucam_Audio_Config failed!\n", MODULE_TAG);
		return -1;
	}
#endif

	/* Step last */
	ret = IF_Ucam_Enable();
	if (ret < 0) {
		printf("ERROR(%s): IF_Ucam_Start failed!\n", MODULE_TAG);
		return -1;
	}

	return 0;
}

int ucamera_system_deinit()
{
	int ret;

	ret = IF_Ucam_Disable();
	if (ret < 0) {
		printf("ERROR(%s): IF_Ucam_Stop failed!\n", MODULE_TAG);
		return -1;
	}

	ret = IF_Ucam_DeInit();
	if (ret < 0) {
		printf("ERROR(%s): IF_Ucam_DeInit failed!\n", MODULE_TAG);
		return -1;
	}

	return 0;

}

/*=============================
 * ucamera control init
 *============================*/
int ucamera_control_init(void* param)
{
	int ret;

	ret = uvc_control_init(param);
	if (ret < 0) {
		printf("ERROR(%s): uvc_control_init failed!\n", MODULE_TAG);
		return -1;
	}

#ifdef MODULE_UAC
	ret = uac_control_init(param);
	if (ret < 0) {
		printf("ERROR(%s): uac_control_init failed!\n", MODULE_TAG);
		return -1;
	}
#endif

	return 0;

}

int ucamera_control_deinit()
{
	uvc_control_deinit();

#ifdef MODULE_UAC
	uac_control_deinit();
#endif
	return 0;

}

/*=============================
 * module ucamera get stream status : ON or OFF
 *============================*/
int module_ucamera_stream_on(void)
{
	return uvc_control_get_stream_status(0);
}

void module_ucamera_get_frame(uint32_t *frame_width, uint32_t *frame_height)
{
	uvc_control_get_frame(frame_width, frame_height);
}

/*=============================
 * module ucamera init
 * ucamera system and control init
 *============================*/
int module_ucamera_init(void *param)
{
	int ret = -1;
	config_func_param_t *cfg_param = (config_func_param_t *)param;

	ret = ucamera_system_init(cfg_param);
	if (ret < 0) {
		printf("ERROR(%s): ucamera_system_init failed!\n", MODULE_TAG);
		return -1;
	}

	usleep(500 * 1000);

	ret = ucamera_control_init(param);
	if (ret < 0) {
		printf("ERROR(%s): ucamera_control_init failed!\n", MODULE_TAG);
		return -1;
	}

	return 0;
}

void module_ucamera_deinit()
{
	ucamera_control_deinit();
	ucamera_system_deinit();
	printf("INFO(%s): module_ucamera_deinit ...ok \n", MODULE_TAG);
}

