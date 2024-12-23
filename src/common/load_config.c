/*********************************************************
 * File Name   : src/common/load_config.c
 * Author      : Tania Xiang
 * Mail        : xiuhui.xiang@ingenic.com
 * Created Time: 2022-05-30 19:48
 ********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <module_config.h>
#include <usbcamera.h>
#include <global_config.h>
#include <imp-common.h>

#define MODULE_TAG            "load_config"

config_func_param_t g_func_param;

static unsigned int get_suitable_payloadsize(int video_num,unsigned int payload_size);
/* start of device_info_init */
int device_param_init(device_info_t* dev_info)
{
	int ret;
	int temp_int; // 使用这个变量去获取int型的数据，防止非int型变量引入地址时发生强转int型导致内存数据出错

	ret = module_config_get_hex("vendor_id", &temp_int);
	if (ret < 0) {
		dev_info->vendor_id = VENDOR_ID;
	} else {
		dev_info->vendor_id = (uint32_t)temp_int;
		printf("INFO[%s]: vendor id 0x%x\n", MODULE_TAG, dev_info->vendor_id);
	}

	ret = module_config_get_hex("device_bcd", &temp_int);
	if (ret < 0) {
		dev_info->dev_bcd = DEVICE_BCD;
	} else {
		dev_info->dev_bcd = (uint32_t)temp_int;
	}

	ret = module_config_get_hex("product_id", &temp_int);
	if (ret < 0) {
		dev_info->product_id = PRODUCT_ID;
	} else {
		dev_info->product_id = (uint32_t)temp_int;
	}

	ret = module_config_get_string("serial_lab", (char*)dev_info->serial_number);
	if (ret < 0) {
		strcpy((char*)dev_info->serial_number, SERIAL_LABEL);
	} else {
		/*printf("INFO[%s]: serial_label %s\n", MODULE_TAG, dev_info->serial_number);*/
	}

	ret = module_config_get_string("product_lab", (char*)dev_info->product_label);
	if (ret < 0) {
		strcpy((char*)dev_info->product_label, PRODUCT_LABEL);
	}
	/*printf("INFO[%s]: product_label %s\n", MODULE_TAG, dev_info->product_label);*/

	ret = module_config_get_string("manufact_lab", (char*)dev_info->manufact_label);
	if (ret < 0) {
		strcpy((char*)dev_info->manufact_label, MANUFACT_LABEL);
	}

	ret = module_config_get_int("adb_en", &temp_int);
	if (ret < 0) {
		dev_info->adb_en = !ADB_ENABLE;
	} else {
		dev_info->adb_en = (uint8_t)temp_int;
	}
	/*printf("INFO[%s]: adb en %d\n", MODULE_TAG, dev_info->adb_en);*/

	return 0;
}
/* end of device_info_init */

/*给视频流分配一个适合的带宽*/
static unsigned int get_suitable_payloadsize(int video_num,unsigned int payload_size)
{
	int i = 0;
	unsigned int payload_size_temp = 0;

	video_info_t *video_info_p = &g_func_param.video_info[video_num];
	if (video_info_p->mps_num == 0 || video_info_p->mps_list == NULL) {
		printf("ERROR:get suitable payload_size err\n");
		return 0;
	}

	for (i = 0; i < video_info_p->mps_num; i++) {
		if (payload_size <= video_info_p->mps_list[i] || i == (video_info_p->mps_num - 1)) {
			payload_size_temp = video_info_p->mps_list[i];
			break;
		}
	}

	return payload_size_temp;
}

/* start of video_info_init */
int config_format(uint32_t fcc, uvc_format_t **format, int frames_num, uint32_t *frame_list, uint8_t fps_num, uint32_t* fps_list)
{
	int i = 0, j = 0, k = 0;
	int ret = 0, temp_int = 0, mult = 0;
	uvc_frame_t *frame = NULL;

	int fps = 0;
	unsigned int image_size = 0;
	unsigned int payload_size = 0;
	int image_compress_ratio = 0;
	int ImageSizeBase = 0;

	if (!*format) {
		/* config format */
		*format = (uvc_format_t*)malloc(sizeof(uvc_format_t));
		if (!*format) {
			printf("ERROR(%s): format malloc failed!\n", MODULE_TAG);
			return -1;
		}
	}

	ret = module_config_get_int("mc", &temp_int);
	if (ret < 0) {
		mult = 2;
	} else {
		mult = mult != 0 ? mult : temp_int;
	}

	ret = module_config_get_int("image_size_base", &temp_int);
	if (ret < 0) {
		ImageSizeBase = 1450000;
	} else {
		if (temp_int == 0) {
			ImageSizeBase = 1450000;
		} else {
			ImageSizeBase = temp_int;
		}
	}

	/*mjpeg h264的带宽分配都是根据1920*1080分辨率图像大小为1450k的数据量作为一个base值*/
	image_compress_ratio = ImageSizeBase * 100 / (1920 * 1080);

	if(fcc == V4L2_PIX_FMT_YUYV || fcc == V4L2_PIX_FMT_NV12) {

		int pos = 0;
		int frames_num_tmp = 0;

		frames_num_tmp = frames_num;
		for(i = 0; i < frames_num; i++) {

			if (frame_list[j] > 640)
				frames_num_tmp--;
			else
				pos = pos == 0 ? j : pos;
			j+=2;
		}
		j=pos;
		frames_num = frames_num_tmp;
	}

	/* config frame init */
	frame = (uvc_frame_t*)malloc(frames_num * sizeof(uvc_frame_t));
	if (!frame) {
		printf("ERROR(%s): frame malloc failed!\n", MODULE_TAG);
		return -1;
	}
/* mult alt setting start */
	for(i = 0; i < frames_num; i++) {
		frame[i].width = (uint16_t)frame_list[j++];
		frame[i].height = (uint16_t)frame_list[j++];
		frame[i].fps_num = fps_num;
		frame[i].fps_list = fps_list;

		frame[i].payload_size_list = (uint32_t *)malloc(fps_num * sizeof(uint32_t));
		if (!frame[i].payload_size_list) {
			 printf("ERROR(%s): payload_size_list malloc failed!\n", MODULE_TAG);
			 return -1;
		}

		if (fcc == V4L2_PIX_FMT_YUYV || fcc == V4L2_PIX_FMT_NV12) {
				for (k = 0; k < fps_num; k++) {
					fps = 10000000 / frame[i].fps_list[k];
					image_size = frame[i].width * frame[i].height * fps * 2;
					payload_size = image_size / 8000;
					frame[i].payload_size_list[k] = get_suitable_payloadsize(0,payload_size);
				}
		} else {
				for (k = 0; k < fps_num; k++) {
					fps = 10000000 / frame[i].fps_list[k];
					image_size = frame[i].width * frame[i].height / 100  * fps * image_compress_ratio;
					payload_size = image_size / 8000;
					frame[i].payload_size_list[k] = get_suitable_payloadsize(0,payload_size);
				}
		}
	}
/* mult alt setting end */

	(*format)->fcc = fcc;
	(*format)->frames_num = frames_num;
	(*format)->uvc_frame = frame;

	return 0;
}

#ifdef DUAL_VIDEO
int config2_format(uint32_t fcc, uvc_format_t **format, int frames_num, uint32_t *frame_list, uint8_t fps_num, uint32_t* fps_list)
{
	int i = 0, j = 0, k = 0;
	int ret = 0, temp_int = 0, mult = 0;
	uvc_frame_t *frame = NULL;

	int fps = 0;
	unsigned int image_size = 0;
	unsigned int payload_size = 0;
	int image_compress_ratio = 0;
	int ImageSizeBase = 0;

	if (!*format) {
		/* config format */
		*format = (uvc_format_t*)malloc(sizeof(uvc_format_t));
		if (!*format) {
			printf("ERROR(%s): format malloc failed!\n", MODULE_TAG);
			return -1;
		}
	}

	ret = module_config_get_int("video2_mc", &temp_int);
	if (ret < 0) {
		mult = 2;
	} else {
		mult = mult != 0 ? mult : temp_int;
	}

	ret = module_config_get_int("video2_image_size_base", &temp_int);
	if (ret < 0) {
		ImageSizeBase = 1450000;
	} else {
		if (temp_int == 0) {
			ImageSizeBase = 1450000;
		} else {
			ImageSizeBase = temp_int;
		}
	}
	/*mjpeg h264的带宽分配都是根据1920*1080分辨率图像大小为1450k的数据量作为一个base值*/
	image_compress_ratio = ImageSizeBase * 100 / (1920 * 1080);

	if(fcc == V4L2_PIX_FMT_YUYV || fcc == V4L2_PIX_FMT_NV12) {

		int pos = 0;
		int frames_num_tmp = 0;

		frames_num_tmp = frames_num;
		for(i = 0; i < frames_num; i++) {

			if (frame_list[j] > 640)
				frames_num_tmp--;
			else
				pos = pos == 0 ? j : pos;
			j+=2;
		}
		j=pos;
		frames_num = frames_num_tmp;
	}

	/* config frame init */
	frame = (uvc_frame_t*)malloc(frames_num * sizeof(uvc_frame_t));
	if (!frame) {
		printf("ERROR(%s): frame malloc failed!\n", MODULE_TAG);
		return -1;
	}
	for(i = 0; i < frames_num; i++) {
		frame[i].width = (uint16_t)frame_list[j++];
		frame[i].height = (uint16_t)frame_list[j++];
		frame[i].fps_num = fps_num;
		frame[i].fps_list = fps_list;

		frame[i].payload_size_list = (uint32_t *)malloc(fps_num * sizeof(uint32_t));
		if (!frame[i].payload_size_list) {
			 printf("ERROR(%s): payload_size_list malloc failed!\n", MODULE_TAG);
			 return -1;
		}

		if (fcc == V4L2_PIX_FMT_YUYV || fcc == V4L2_PIX_FMT_NV12) {
				for (k = 0; k < fps_num; k++) {
					fps = 10000000 / frame[i].fps_list[k];
					image_size = frame[i].width * frame[i].height * fps * 2;
					payload_size = image_size / 8000;
					frame[i].payload_size_list[k] = get_suitable_payloadsize(1,payload_size);
				}
		} else {
				for (k = 0; k < fps_num; k++) {
					fps = 10000000 / frame[i].fps_list[k];
					image_size = frame[i].width * frame[i].height / 100  * fps * image_compress_ratio;
					payload_size = image_size / 8000;
					frame[i].payload_size_list[k] = get_suitable_payloadsize(1,payload_size);
				}
		}
	}

	(*format)->fcc = fcc;
	(*format)->frames_num = frames_num;
	(*format)->uvc_frame = frame;

	return 0;
}
#endif
/* uvc format init */
/* init supported format */
/* init supported frames num and frames list */
/* init supported fps num and fps list */
int uvc_format_init(video_info_t* video_info)
{
	int ret;
	uint8_t format_num = 0;
	uvc_format_t *yuv_format = NULL;
	uvc_format_t *yuyv_format = NULL;
	uvc_format_t *mjpeg_format = NULL;
	uvc_format_t *h264_format = NULL;
	uvc_format_t *h265_format = NULL;
	uvc_format_t *uvc_format_tmp[UVC_FORMAT_MAX]; /* adjust the order of uvc_format */

	uint32_t frames_num = 0;
	uint32_t *frame_list = NULL;
	uint32_t fps_num = 0;
	uint32_t *fps_list = NULL;
	int i = 0;

	/* get the number of supported fps */
	ret = module_config_get_int("fps_num", (int*)&fps_num);
	if (ret < 0) {
		fps_num = 1;
	}

	/* get fps_list ,such as 60,40, 30, 25 etc. */
	fps_list = (uint32_t*)malloc(fps_num * sizeof(uint32_t));
	if (!fps_list) {
		printf("ERROR(%s): fps_list malloc failed!\n", MODULE_TAG);
		return -1;
	}

	ret = module_config_get_mult_fps("fps_list", fps_list);
	if (ret < 0) {
		printf("ERROR(%s): fps_list get failed!\n", MODULE_TAG);
		return -1;
	}

	/* get the number of supported frames */
	ret = module_config_get_int("nframes", (int*)&frames_num);
	if (ret < 0) {
		frames_num = 1;
	}
	printf("frame num %d\n", frames_num);

	/* get frame list : frames_num * (width + height)*/
	frame_list = (uint32_t *)malloc(frames_num * 2 * sizeof(uint32_t));
	if (!frame_list) {
		printf("ERROR(%s): frame list malloc failed!\n", MODULE_TAG);
		return -1;
	}

	ret = module_config_get_mult_frames(frames_num, frame_list);
	if (ret < 0) {
		printf("ERROR(%s): frame_list get failed!\n", MODULE_TAG);
		return -1;
	}

	printf("---- %d, %d, %d, %d, %d, %d -----\n",frame_list[0], frame_list[1], frame_list[2],frame_list[3],frame_list[4],frame_list[5]);
	if (video_info->yuv_en) {
		ret = config_format(V4L2_PIX_FMT_NV12, &yuv_format, frames_num, frame_list, fps_num, fps_list);
		if (ret < 0) {
			printf("ERROR(%s): config format failed!\n", MODULE_TAG);
			return -1;
		}
		uvc_format_tmp[video_info->yuv_en - 1] = yuv_format;
		format_num ++;
	}

	if (video_info->yuyv_en) {
		ret = config_format(V4L2_PIX_FMT_YUYV, &yuyv_format, frames_num, frame_list, fps_num, fps_list);
		if (ret < 0) {
			printf("ERROR(%s): config format failed!\n", MODULE_TAG);
			return -1;
		}
		uvc_format_tmp[video_info->yuyv_en - 1] = yuyv_format;
		format_num ++;
	}

	if (video_info->mjpeg_en) {
		ret = config_format(V4L2_PIX_FMT_MJPEG, &mjpeg_format, frames_num, frame_list, fps_num, fps_list);
		if (ret < 0) {
			printf("ERROR(%s): config format failed!\n", MODULE_TAG);
			return -1;
		}
		uvc_format_tmp[video_info->mjpeg_en - 1] = mjpeg_format;
		format_num ++;
	}

	if (video_info->h264_en) {
		ret = config_format(V4L2_PIX_FMT_H264, &h264_format, frames_num, frame_list, fps_num, fps_list);
		if (ret < 0) {
			printf("ERROR(%s): config format failed!\n", MODULE_TAG);
			return -1;
		}
		uvc_format_tmp[video_info->h264_en - 1] = h264_format;
		format_num ++;
	}

	if (video_info->h265_en) {
		ret = config_format(V4L2_PIX_FMT_H265, &h265_format, frames_num, frame_list, fps_num, fps_list);
		if (ret < 0) {
			printf("ERROR(%s): config format failed!\n", MODULE_TAG);
			return -1;
		}
		uvc_format_tmp[video_info->h265_en - 1] = h265_format;
		format_num ++;
	}

	/* set uvc format list */
	video_info->uvc_format_num = format_num;
	video_info->uvc_format = (uvc_format_t**)malloc(video_info->uvc_format_num * sizeof(uvc_format_t*));
	if (!video_info->uvc_format) {
		printf("ERROR(%s): uvc_format malloc failed!\n", MODULE_TAG);
		return -1;
	}

	for (i = 0; i < format_num; i++) {
		video_info->uvc_format[i] = uvc_format_tmp[i];
	}

	free(frame_list);

	return 0;
}

int uvc_vc_config(video_info_t* video_info)
{
	int ret;
	uint8_t *temp_int = NULL;
	temp_int = (uint8_t *)malloc(16*sizeof(uint8_t));
	int temp; // 使用这个变量去获取int型的数据，防止非int型变量引入地址时发生强转int型导致内存数据出错

	ret = module_config_get_uhex("ct_controls", temp_int,sizeof(video_info->uvc_ct.controls));
	if (ret < 0) {
		video_info->uvc_ct.controls[0] = (uint8_t)0xa;
		video_info->uvc_ct.controls[1] = (uint8_t)0x2;
		video_info->uvc_ct.controls[2] = (uint8_t)0x0;
		/*printf("INFO[CT]sucess !!!\n");*/
	}else{
		memcpy(video_info->uvc_ct.controls,temp_int,sizeof(video_info->uvc_ct.controls));
		/*printf("uvc CT config: %hhu %hhu %hhu\n",video_info->uvc_ct.controls[0]\*/
				/*,video_info->uvc_ct.controls[1],\*/
				/*video_info->uvc_ct.controls[2]);*/
	}

	ret = module_config_get_uhex("pu_controls", temp_int,sizeof(video_info->uvc_pu.controls));
	if (ret < 0) {
		video_info->uvc_pu.controls[0] = (uint8_t)0xff;
		video_info->uvc_pu.controls[1] = (uint8_t)0x15;
#ifdef CONFIG_UVC_1_5
		video_info->uvc_pu.controls[2] = (uint8_t)0x77;
#endif
		/*printf("INFO[PU]sucess !!!\n");*/
	}else{
		memcpy(video_info->uvc_pu.controls,temp_int,sizeof(video_info->uvc_pu.controls));
		/*printf("uvc PU config: %hhu %hhu %hhu\n",video_info->uvc_pu.controls[0]\*/
				/*,video_info->uvc_pu.controls[1],\*/
				/*temp_int[2]);*/
	}

	ret = module_config_get_int("eu_en", &temp);
	if (ret < 0) {
		video_info->uvc_eu.en = (uint8_t)0x00;
		/*printf("INFO[EU]en sucess !!!\n");*/
	}else{
		video_info->uvc_eu.en = temp;
		/*printf("INFO[EU]en:%hhu \n",temp_int[0]);*/
	}

	ret = module_config_get_int("eu_num", &temp);
	if (ret < 0) {
		video_info->uvc_eu.num_controls = (uint8_t)0xF;
		/*printf("INFO[EU]num sucess !!!\n");*/
	}else{
		video_info->uvc_eu.num_controls = temp;
		/*printf("INFO[EU]num:%hhu \n",temp_int[0]);*/
	}

	uint8_t def_guid[16] = {0xa2, 0x9e, 0x76, 0x41, 0xde, 0x04, 0x47, 0xe3,0x8b, 0x2b, 0xf4, 0x34, 0x1a, 0xff, 0x00, 0x3b};
	ret = module_config_get_uhex("eu_guid", temp_int,sizeof(video_info->uvc_eu.guid));
	if (ret < 0) {
		memcpy(video_info->uvc_eu.guid,def_guid,sizeof(video_info->uvc_eu.guid));
		/*printf("INFO[EU]guid sucess !!!\n");*/
	}else{
		memcpy(video_info->uvc_eu.guid,temp_int,sizeof(video_info->uvc_eu.guid));
		/*printf("INFO[EU]guid 16:%hhu \n",temp_int[15]);*/
	}

	ret = module_config_get_uhex("eu_controls",temp_int,sizeof(video_info->uvc_eu.controls));
	if (ret < 0) {
		video_info->uvc_eu.controls[0] = (uint8_t)0x00;
		video_info->uvc_eu.controls[1] = (uint8_t)0xf8;
		video_info->uvc_eu.controls[2] = (uint8_t)0x03;
		/*printf("INFO[EU]controls sucess !!!\n");*/
	}else{

		memcpy(video_info->uvc_eu.controls,temp_int,sizeof(video_info->uvc_eu.controls));
		/*printf("uvc EU config: %hhu %hhu %hhu\n",video_info->uvc_eu.controls[0]\*/
				/*,video_info->uvc_eu.controls[1],\*/
				/*video_info->uvc_eu.controls[2]);*/
	}

	printf("INFO[%s] --> vc config success <--\n", MODULE_TAG);
	free(temp_int);

	return 0;
};

int video_param_init(video_info_t* video_info)
{
	int ret;
	int temp_int; // 使用这个变量去获取int型的数据，防止非int型变量引入地址时发生强转int型导致内存数据出错

	ret = module_config_get_string("video_name", (char*)video_info->video_name);
	if (ret < 0) {
		strcpy((char*)video_info->video_name, VIDEO_DEVICE_NAME);
	}

	ret = module_config_get_int("mc", &temp_int);
	if (ret < 0) {
		video_info->mult = 2;
	} else {
		video_info->mult = (uint8_t)temp_int;
	}

	ret = module_config_get_int("mps", &temp_int);
	if (ret < 0) {
		video_info->mps = 1024;
	} else {
		video_info->mps = temp_int;
	}

	/* get the number of supported maxpacketsize */
	ret = module_config_get_int("mps_num", &temp_int);
	if (ret < 0) {
		video_info->mps_num = 11;
	} else {
		if (temp_int > 11) {
			printf("ERROR:mps_num > 11,please check uvc.config");
			return 0;
		}
		video_info->mps_num = temp_int;
	}

	/* get mps_list ,such as  3072, 2048, 1024 etc. */
	video_info->mps_list = (uint32_t*)malloc(video_info->mps_num * sizeof(uint32_t));
	if (!video_info->mps_list) {
		printf("ERROR(%s): mps_list malloc failed!\n", MODULE_TAG);
		return -1;
	}

	ret = module_config_get_mult_maxpacksize("mps_list", video_info->mps_list);
	if (ret < 0) {
		printf("Warning(%s): mps_list get failed, use default settings!\n", MODULE_TAG);
		video_info->mps_num = 11;
		video_info->mps_list[0] = 192;
		video_info->mps_list[1] = 384;
		video_info->mps_list[2] = 512;
		video_info->mps_list[3] = 640;
		video_info->mps_list[4] = 800;
		video_info->mps_list[5] = 944;
		video_info->mps_list[6] = 1280;
		video_info->mps_list[7] = 1600;
		video_info->mps_list[8] = 1984;
		video_info->mps_list[9] = 2688;
		video_info->mps_list[10] = 3072;
	}

	/*根据mc mps算出最大带宽，如果带宽列表中的值大于最大带宽，修改带宽列表的值为最大带宽*/
	for (int i = 0; i < video_info->mps_num; i++) {
		if((video_info->mps * (video_info->mult + 1)) <= video_info->mps_list[i]) {
			video_info->mps_list[i] = video_info->mps * (video_info->mult + 1);

			if (i != video_info->mps_num -1) {
				int AbnormalPayloadSizeNum = video_info->mps_num - (i + 1);
				video_info->mps_num -= AbnormalPayloadSizeNum;
				break;
			}
		}

	}

	ret = module_config_get_int("stillcap_en", &temp_int);
	if (ret < 0) {
		video_info->stillcap_en = 1;
	} else {
		video_info->stillcap_en = (uint8_t)temp_int;
	}

	ret = module_config_get_int("h264_en", &temp_int);
	if (ret < 0) {
		video_info->h264_en = 0;
	} else {
		video_info->h264_en = (uint8_t)temp_int;
	}

	ret = module_config_get_int("h265_en", &temp_int);
	if (ret < 0) {
		video_info->h265_en = 0;
	} else {
		video_info->h265_en = (uint8_t)temp_int;
	}

	ret = module_config_get_int("nv12_en", &temp_int);
	if (ret < 0) {
		video_info->yuv_en = 0;
	} else {
		video_info->yuv_en = (uint8_t)temp_int;
	}

	ret = module_config_get_int("yuyv_en", &temp_int);
	if (ret < 0) {
		video_info->yuyv_en = 1;
	} else {
		video_info->yuyv_en = (uint8_t)temp_int;
	}

	ret = module_config_get_int("mjpeg_en", &temp_int);
	if (ret < 0) {
		video_info->mjpeg_en = 1;
	} else {
		video_info->mjpeg_en = (uint8_t)temp_int;
	}

	/* led_ctr */
	ret = module_config_get_int("led_en", &temp_int);
	if (ret < 0) {
		video_info->led_ctl.led_en = 0;
	} else {
		video_info->led_ctl.led_en = (uint8_t)temp_int;
	}

	ret = module_config_get_int("led_gpio", &temp_int);
	if (ret < 0) {
		video_info->led_ctl.led_gpio = -1;
	} else {
		video_info->led_ctl.led_gpio = (uint8_t)temp_int;
	}

	ret = module_config_get_int("led_level", &temp_int);
	if (ret < 0) {
		video_info->led_ctl.led_level = 0;
	} else {
		video_info->led_ctl.led_level = (uint8_t)temp_int;
	}

	/* init uvc_format such as yuv ,mjpeg, h264 etc. */
	ret = uvc_format_init(video_info);
	if (ret < 0) {
		printf("ERROR(%s): uvc_format_init failed!\n", MODULE_TAG);
		return -1;
	}
	/* uvc vc init*/
	ret = uvc_vc_config(video_info);
	if (ret < 0) {
		printf("ERROR(%s): uvc_vc_config failed!\n", MODULE_TAG);
		return -1;
	}

	return 0;
}
#if (VIDEO_NUM == 2)
/* uvc format init */
/* init supported format */
/* init supported frames num and frames list */
/* init supported fps num and fps list */
int uvc2_format_init(video_info_t* video_info)
{
	int ret;
	uint8_t format_num = 0;
	uvc_format_t *yuv_format = NULL;
	uvc_format_t *yuyv_format = NULL;
	uvc_format_t *mjpeg_format = NULL;
	uvc_format_t *h264_format = NULL;
	uvc_format_t *h265_format = NULL;
	uvc_format_t *uvc_format_tmp[UVC_FORMAT_MAX]; /* adjust the order of uvc_format */

	uint32_t frames_num = 0;
	uint32_t *frame_list = NULL;
	uint32_t fps_num = 0;
	uint32_t *fps_list = NULL;
	int i = 0;

	/* get the number of supported fps */
	ret = module_config_get_int("video2_fps_num", (int*)&fps_num);
	if (ret < 0) {
		fps_num = 1;
	}

	/* get fps_list ,such as 60,40, 30, 25 etc. */
	fps_list = (uint32_t*)malloc(fps_num * sizeof(uint32_t));
	if (!fps_list) {
		printf("ERROR(%s): fps_list malloc failed!\n", MODULE_TAG);
		return -1;
	}

	ret = module_config_get_mult_fps("video2_fps_list", fps_list);
	if (ret < 0) {
		printf("ERROR(%s): video2_fps_list get failed!\n", MODULE_TAG);
		return -1;
	}

	/* get the number of supported frames */
	ret = module_config_get_int("video2_nframes", (int*)&frames_num);
	if (ret < 0) {
		frames_num = 1;
	}
	printf("video2 frame num %d\n", frames_num);

	/* get frame list : frames_num * (width + height)*/
	frame_list = (uint32_t *)malloc(frames_num * 2 * sizeof(uint32_t));
	if (!frame_list) {
		printf("ERROR(%s): frame list malloc failed!\n", MODULE_TAG);
		return -1;
	}

	ret = module_config_get_mult_frames(frames_num, frame_list);
	if (ret < 0) {
		printf("ERROR(%s): frame_list get failed!\n", MODULE_TAG);
		return -1;
	}

	printf("----video2: %d, %d, %d, %d, %d, %d -----\n",frame_list[0], frame_list[1], frame_list[2],frame_list[3],frame_list[4],frame_list[5]);
	if (video_info->yuv_en) {
		ret = config2_format(V4L2_PIX_FMT_NV12, &yuv_format, frames_num, frame_list, fps_num, fps_list);
		if (ret < 0) {
			printf("ERROR(%s): config format failed!\n", MODULE_TAG);
			return -1;
		}
		uvc_format_tmp[video_info->yuv_en - 1] = yuv_format;
		format_num ++;
	}

	if (video_info->yuyv_en) {

#ifdef WINHELLO_ENABLE
		if(video_info->yuv_en || video_info->mjpeg_en || video_info->h264_en || video_info->h265_en) {
			printf("ERROR(%s): fatal error, winhello only supported yuv(y) format\n", MODULE_TAG);
			return -1;
		}

		ret = config2_format(V4L2_PIX_FMT_GREY, &yuyv_format, frames_num, frame_list, fps_num, fps_list);
		if (ret < 0) {
			printf("ERROR(%s): config format failed!\n", MODULE_TAG);
			return -1;
		}
		uvc_format_tmp[video_info->yuyv_en - 1] = yuyv_format;
#else
		ret = config2_format(V4L2_PIX_FMT_YUYV, &yuyv_format, frames_num, frame_list, fps_num, fps_list);
		if (ret < 0) {
			printf("ERROR(%s): config format failed!\n", MODULE_TAG);
			return -1;
		}
		uvc_format_tmp[video_info->yuyv_en - 1] = yuyv_format;
#endif
		format_num ++;
	}

	if (video_info->mjpeg_en) {
		ret = config2_format(V4L2_PIX_FMT_MJPEG, &mjpeg_format, frames_num, frame_list, fps_num, fps_list);
		if (ret < 0) {
			printf("ERROR(%s): config format failed!\n", MODULE_TAG);
			return -1;
		}
		uvc_format_tmp[video_info->mjpeg_en - 1] = mjpeg_format;
		format_num ++;
	}

	if (video_info->h264_en) {
		ret = config2_format(V4L2_PIX_FMT_H264, &h264_format, frames_num, frame_list, fps_num, fps_list);
		if (ret < 0) {
			printf("ERROR(%s): config format failed!\n", MODULE_TAG);
			return -1;
		}
		uvc_format_tmp[video_info->h264_en - 1] = h264_format;
		format_num ++;
	}

	if (video_info->h265_en) {
		ret = config2_format(V4L2_PIX_FMT_H265, &h265_format, frames_num, frame_list, fps_num, fps_list);
		if (ret < 0) {
			printf("ERROR(%s): config format failed!\n", MODULE_TAG);
			return -1;
		}
		uvc_format_tmp[video_info->h265_en - 1] = h265_format;
		format_num ++;
	}

	/* set uvc format list */
	video_info->uvc_format_num = format_num;
	video_info->uvc_format = (uvc_format_t**)malloc(video_info->uvc_format_num * sizeof(uvc_format_t*));
	if (!video_info->uvc_format) {
		printf("ERROR(%s): uvc_format malloc failed!\n", MODULE_TAG);
		return -1;
	}

	for (i = 0; i < format_num; i++) {
		video_info->uvc_format[i] = uvc_format_tmp[i];
	}

	free(frame_list);

	return 0;
}

int uvc2_vc_config(video_info_t* video_info)
{
	int ret;
	uint8_t *temp_int = NULL;
	temp_int = (uint8_t *)malloc(16*sizeof(uint8_t));
	int temp; // 使用这个变量去获取int型的数据，防止非int型变量引入地址时发生强转int型导致内存数据出错

	ret = module_config_get_uhex("ct2_controls", temp_int,sizeof(video_info->uvc_ct.controls));
	if (ret < 0) {
		video_info->uvc_ct.controls[0] = (uint8_t)0xa;
		video_info->uvc_ct.controls[1] = (uint8_t)0x2;
		video_info->uvc_ct.controls[2] = (uint8_t)0x0;
		/*printf("INFO[CT2]sucess !!!\n");*/
	}else{
		memcpy(video_info->uvc_ct.controls,temp_int,sizeof(video_info->uvc_ct.controls));
		/*printf("uvc2 CT config: %hhu %hhu %hhu\n",video_info->uvc_ct.controls[0]\*/
				/*,video_info->uvc_ct.controls[1],\*/
				/*video_info->uvc_ct.controls[2]);*/
	}

	ret = module_config_get_uhex("pu2_controls", temp_int,sizeof(video_info->uvc_pu.controls));
	if (ret < 0) {
		video_info->uvc_pu.controls[0] = (uint8_t)0xff;
		video_info->uvc_pu.controls[1] = (uint8_t)0x15;
#ifdef CONFIG_UVC_1_5
		video_info->uvc_pu.controls[2] = (uint8_t)0x77;
#endif
		/*printf("INFO[PU2]sucess !!!\n");*/
	}else{
		memcpy(video_info->uvc_pu.controls,temp_int,sizeof(video_info->uvc_pu.controls));
		/*printf("uvc2 PU config: %hhu %hhu %hhu\n",video_info->uvc_pu.controls[0]\*/
				/*,video_info->uvc_pu.controls[1],\*/
				/*temp_int[2]);*/
	}

	ret = module_config_get_int("eu2_en", &temp);
	if (ret < 0) {
		video_info->uvc_eu.en = (uint8_t)0x00;
		/*printf("INFO[EU2]en sucess !!!\n");*/
	}else{
		video_info->uvc_eu.en = temp;
		/*printf("INFO[EU2]en:%hhu \n",temp_int[0]);*/
	}

	ret = module_config_get_int("eu2_num", &temp);
	if (ret < 0) {
		video_info->uvc_eu.num_controls = (uint8_t)0xF;
		/*printf("INFO[EU2]num sucess !!!\n");*/
	}else{
		video_info->uvc_eu.num_controls = temp;
		/*printf("INFO[EU2]num:%hhu \n",temp_int[0]);*/
	}

	uint8_t def_guid[16] = {0xa2, 0x9e, 0x76, 0x41, 0xde, 0x04, 0x47, 0xe3,0x8b, 0x2b, 0xf4, 0x34, 0x1a, 0xff, 0x00, 0x3b};
	ret = module_config_get_uhex("eu2_guid", temp_int,sizeof(video_info->uvc_eu.guid));
	if (ret < 0) {
		memcpy(video_info->uvc_eu.guid,def_guid,sizeof(video_info->uvc_eu.guid));
		/*printf("INFO[EU2]guid sucess !!!\n");*/
	}else{
		memcpy(video_info->uvc_eu.guid,temp_int,sizeof(video_info->uvc_eu.guid));
		/*printf("INFO[EU2]guid 16:%hhu \n",temp_int[15]);*/
	}

	ret = module_config_get_uhex("eu2_controls",temp_int,sizeof(video_info->uvc_eu.controls));
	if (ret < 0) {
		video_info->uvc_eu.controls[0] = (uint8_t)0x00;
		video_info->uvc_eu.controls[1] = (uint8_t)0xf8;
		video_info->uvc_eu.controls[2] = (uint8_t)0x03;
		/*printf("INFO[EU2]controls sucess !!!\n");*/
	}else{

		memcpy(video_info->uvc_eu.controls,temp_int,sizeof(video_info->uvc_eu.controls));
		/*printf("uvc2 EU config: %hhu %hhu %hhu\n",video_info->uvc_eu.controls[0]\*/
				/*,video_info->uvc_eu.controls[1],\*/
				/*video_info->uvc_eu.controls[2]);*/
	}

	printf("INFO[VC2 CONIFG] success !\n");
	free(temp_int);

	return 0;
};
int video2_param_init(video_info_t* video_info)
{
	int ret;
	int temp_int; // 使用这个变量去获取int型的数据，防止非int型变量引入地址时发生强转int型导致内存数据出错

	ret = module_config_get_string("video2_name", (char*)video_info->video_name);
	if (ret < 0) {
		strcpy((char*)video_info->video_name, VIDEO_DEVICE_NAME);
	}

	ret = module_config_get_int("video2_mc", &temp_int);
	if (ret < 0) {
		video_info->mult = 1;
	} else {
		video_info->mult = (uint8_t)temp_int;
	}

	ret = module_config_get_int("video2_mps", &temp_int);
	if (ret < 0) {
		video_info->mps = 1024;
	} else {
		video_info->mps = temp_int;
	}

	/* get the number of supported maxpacketsize */
	ret = module_config_get_int("video2_mps_num", &temp_int);
	if (ret < 0) {
		video_info->mps_num = 11;
	} else {
		if (temp_int > 11) {
			printf("ERROR:mps_num > 11,please check uvc.config");
			return 0;
		}
		video_info->mps_num = temp_int;
	}

	/* get mps_list ,such as  3072, 2048, 1024 etc. */
	video_info->mps_list = (uint32_t*)malloc(video_info->mps_num * sizeof(uint32_t));
	if (!video_info->mps_list) {
		printf("ERROR(%s): mps_list malloc failed!\n", MODULE_TAG);
		return -1;
	}

	ret = module_config_get_mult_maxpacksize("video2_mps_list", video_info->mps_list);
	if (ret < 0) {
		printf("Warning(%s): video2_mps_list get failed, use default settings!\n", MODULE_TAG);
		video_info->mps_num = 11;
		video_info->mps_list[0] = 192;
		video_info->mps_list[1] = 384;
		video_info->mps_list[2] = 512;
		video_info->mps_list[3] = 640;
		video_info->mps_list[4] = 800;
		video_info->mps_list[5] = 944;
		video_info->mps_list[6] = 1280;
		video_info->mps_list[7] = 1600;
		video_info->mps_list[8] = 1984;
		video_info->mps_list[9] = 2688;
		video_info->mps_list[10] = 3072;
	}

	/*根据mc mps算出最大带宽，如果带宽列表中的值大于最大带宽，修改带宽列表的值为最大带宽*/
	for (int i = 0; i < video_info->mps_num; i++) {
		if(video_info->mps * (video_info->mult + 1) <= video_info->mps_list[i]) {
			video_info->mps_list[i] = video_info->mps * (video_info->mult + 1);

			if (i != video_info->mps_num -1) {
				printf("warning:uvc2:payload_size value is larger than MaxPayloadSize\n");
				int AbnormalPayloadSizeNum = video_info->mps_num - (i + 1);
				video_info->mps_num -= AbnormalPayloadSizeNum;
				break;
			}
		}
	}

	ret = module_config_get_int("video2_stillcap_en", &temp_int);
	if (ret < 0) {
		video_info->stillcap_en = 1;
	} else {
		video_info->stillcap_en = (uint8_t)temp_int;
	}

	ret = module_config_get_int("video2_h264_en", &temp_int);
	if (ret < 0) {
		video_info->h264_en = 0;
	} else {
		video_info->h264_en = (uint8_t)temp_int;
	}

	ret = module_config_get_int("video2_h265_en", &temp_int);
	if (ret < 0) {
		video_info->h265_en = 0;
	} else {
		video_info->h265_en = (uint8_t)temp_int;
	}

	ret = module_config_get_int("video2_nv12_en", &temp_int);
	if (ret < 0) {
		video_info->yuv_en = 1;
	} else {
		video_info->yuv_en = (uint8_t)temp_int;
	}

	ret = module_config_get_int("video2_yuyv_en", &temp_int);
	if (ret < 0) {
		video_info->yuyv_en = 1;
	} else {
		video_info->yuyv_en = (uint8_t)temp_int;
	}

	ret = module_config_get_int("video2_mjpeg_en", &temp_int);
	if (ret < 0) {
		video_info->mjpeg_en = 1;
	} else {
		video_info->mjpeg_en = (uint8_t)temp_int;
	}

	/* led_ctr */
	ret = module_config_get_int("led2_en", &temp_int);
	if (ret < 0) {
		video_info->led_ctl.led_en = 0;
	} else {
		video_info->led_ctl.led_en = (uint8_t)temp_int;
	}

	ret = module_config_get_int("led2_gpio", &temp_int);
	if (ret < 0) {
		video_info->led_ctl.led_gpio = -1;
	} else {
		video_info->led_ctl.led_gpio = (uint8_t)temp_int;
	}

	ret = module_config_get_int("led2_level", &temp_int);
	if (ret < 0) {
		video_info->led_ctl.led_level = 0;
	} else {
		video_info->led_ctl.led_level = (uint8_t)temp_int;
	}

	/*winhello_en*/
	ret = module_config_get_int("winhello_en", &temp_int);
	if (ret < 0) {
		video_info->winhello_en = 0;
	} else {
		video_info->winhello_en = (uint8_t)temp_int;
	}

	/* init uvc_format such as yuv ,mjpeg, h264 etc. */
	ret = uvc2_format_init(video_info);
	if (ret < 0) {
		printf("ERROR(%s): uvc2_format_init failed!\n", MODULE_TAG);
		return -1;
	}
	/* uvc2 vc init*/
	ret = uvc2_vc_config(video_info);
	if (ret < 0) {
		printf("ERROR(%s): uvc2_vc_config failed!\n", MODULE_TAG);
		return -1;
	}

	return 0;
}
#endif
/* end of video_info_init */

/* start of audio_info_init */
int amic_info_init(audio_param_info_t *mic_info)
{
	int ret;
	int temp_int; // 使用这个变量去获取int型的数据，防止非int型变量引入地址时发生强转int型导致内存数据出错

	ret = module_config_get_string("amic_name", (char*)mic_info->audio_name);
	if (ret < 0) {
		strcpy((char*)mic_info->audio_name, AUDIO_DEVICE_NAME);;
	}

	ret = module_config_get_int("amic_samplerate", &temp_int);
	if (ret < 0) {
		mic_info->samplerate = SAMPLERATE_DEF;
	} else {
		mic_info->samplerate = (uint32_t)temp_int;
	}
	printf("INFO[%s]: amic samplerate %d\n", MODULE_TAG, mic_info->samplerate);

	ret = module_config_get_int("amic_bitwidth", &temp_int);
	if (ret < 0) {
		mic_info->bitwidth = BITWIDTH_DEF;
	} else {
		mic_info->bitwidth = (uint8_t)temp_int;
	}

	ret = module_config_get_int("amic_soundmode", &temp_int);
	if (ret < 0) {
		mic_info->soundmode = SOUNDMODE_DEF;
	} else {
		mic_info->soundmode = (uint8_t)temp_int;
	}
	printf("INFO[%s]: amic soundmode %d\n", MODULE_TAG, mic_info->soundmode);

	ret = module_config_get_int("amic_volume", &temp_int);
	if (ret < 0) {
		mic_info->volume = MIC_VOL;
	} else {
		mic_info->volume = (int8_t)temp_int;
	}

	ret = module_config_get_int("amic_chncnt", &temp_int);
	if (ret < 0) {
		mic_info->prev[0] = MIC_CHNCNT;
	} else{
		mic_info->prev[0] = (int8_t)temp_int;
	}
	return 0;
}

int dmic_info_init(audio_param_info_t *mic_info)
{
	int ret;
	int temp_int; // 使用这个变量去获取int型的数据，防止非int型变量引入地址时发生强转int型导致内存数据出错

	ret = module_config_get_string("dmic_name", (char*)mic_info->audio_name);
	if (ret < 0) {
		strcpy((char*)mic_info->audio_name, AUDIO_DEVICE_NAME);;
	}

	ret = module_config_get_int("dmic_samplerate", &temp_int);
	if (ret < 0) {
		mic_info->samplerate = SAMPLERATE_DEF;
	} else {
		mic_info->samplerate = (uint32_t)temp_int;
	}

	ret = module_config_get_int("dmic_bitwidth", &temp_int);
	if (ret < 0) {
		mic_info->bitwidth = BITWIDTH_DEF;
	} else {
		mic_info->bitwidth = (uint8_t)temp_int;
	}

	ret = module_config_get_int("dmic_soundmode", &temp_int);
	if (ret < 0) {
		mic_info->soundmode = SOUNDMODE_DEF;
	} else {
		mic_info->soundmode = (uint8_t)temp_int;
	}

	ret = module_config_get_int("dmic_volume", &temp_int);
	if (ret < 0) {
		mic_info->volume = MIC_VOL;
	} else {
		mic_info->volume = (int8_t)temp_int;
	}

	ret = module_config_get_int("dmic_chncnt", &temp_int);
	if (ret < 0) {
		mic_info->prev[0] = MIC_CHNCNT;
	} else {
		mic_info->prev[0] = (int8_t)temp_int;
	}
	return 0;
}
int spk_info_init(audio_param_info_t *spk_info)
{
	int ret;
	int temp_int; // 使用这个变量去获取int型的数据，防止非int型变量引入地址时发生强转int型导致内存数据出错

	ret = module_config_get_string("spk_name", (char*)spk_info->audio_name);
	if (ret < 0) {
		strcpy((char*)spk_info->audio_name, AUDIO_DEVICE_NAME);;
	}

	ret = module_config_get_int("spk_samplerate", &temp_int);
	if (ret < 0) {
		spk_info->samplerate = SAMPLERATE_DEF;
	} else {
		spk_info->samplerate = (uint32_t)temp_int;
	}

	ret = module_config_get_int("spk_bitwidth", &temp_int);
	if (ret < 0) {
		spk_info->bitwidth = BITWIDTH_DEF;
	} else {
		spk_info->bitwidth = (uint8_t)temp_int;
	}

	ret = module_config_get_int("spk_soundmode", &temp_int);
	if (ret < 0) {
		spk_info->soundmode = SOUNDMODE_DEF;
	} else {
		spk_info->soundmode = (uint8_t)temp_int;
	}

	ret = module_config_get_int("spk_volume", &temp_int);
	if (ret < 0) {
		spk_info->volume = MIC_VOL;
	} else {
		spk_info->volume = (int8_t)temp_int;
	}

	return 0;
}

int audio_param_init(audio_info_t* audio_info)
{
	int ret;
	int temp_int; // 使用这个变量去获取int型的数据，防止非int型变量引入地址时发生强转int型导致内存数据出错

	ret = module_config_get_int("amic_en", &temp_int);
	if (ret < 0) {
		audio_info->amic_en = 1;
	} else {
		audio_info->amic_en = (uint8_t)temp_int;
	}

	ret = module_config_get_int("dmic_en", &temp_int);
	if (ret < 0) {
		audio_info->dmic_en = 0;
	} else {
		audio_info->dmic_en = (uint8_t)temp_int;
	}

	ret = module_config_get_int("spk_en", &temp_int);
	if (ret < 0) {
		audio_info->spk_en = 1;
	} else {
		audio_info->spk_en = (uint8_t)temp_int;
	}

	if (audio_info->amic_en) {
		ret = amic_info_init(&audio_info->amic_info);
		if (ret < 0) {
			printf("ERROR(%s): amic info init failed!\n", MODULE_TAG);
			return -1;
		}
	}

	if (audio_info->dmic_en) {
		ret = dmic_info_init(&audio_info->dmic_info);
		if (ret < 0) {
			printf("ERROR(%s): dmic info init failed!\n", MODULE_TAG);
			return -1;
		}
	}

	if (audio_info->spk_en) {
		ret = spk_info_init(&audio_info->spk_info);
		if (ret < 0) {
			printf("ERROR(%s): spk info init failed!\n", MODULE_TAG);
			return -1;
		}
	}

	ret = module_config_get_int("aec_en", &temp_int);
	if (ret < 0) {
		audio_info->aec_en = 0;
	} else {
		audio_info->aec_en = (uint8_t)temp_int;
	}

	ret = module_config_get_int("audio_ns", &temp_int);
	if (ret < 0) {
		audio_info->audio_ns = 0;
	} else {
		audio_info->audio_ns = (uint8_t)temp_int;
	}

	return 0;
}
/* end of audio_info_init */

/* start of imp_info_init */
int sensor_info_init(sensor_info_t *sensor_info)
{
	int ret;
	int temp_int; // 使用这个变量去获取int型的数据，防止非int型变量引入地址时发生强转int型导致内存数据出错

	ret = module_config_get_string("sensor_name", (char*)sensor_info->sensor_name);
	if (ret < 0) {
		strcpy((char*)sensor_info->sensor_name, FIRST_SENSOR_NAME);
	}

	ret = module_config_get_hex("i2c_addr", &temp_int);
	if (ret < 0) {
		sensor_info->i2c_addr = FIRST_I2C_ADDR;
	} else {
		sensor_info->i2c_addr = (uint16_t)temp_int;
	}

	ret = module_config_get_int("default_boot", &temp_int);
	if (ret < 0) {
		sensor_info->default_boot = FIRST_DEFAULT_BOOT;
	} else {
		sensor_info->default_boot = (uint8_t)temp_int;
	}

	ret = module_config_get_int("sensor_width", &temp_int);
	if (ret < 0) {
		sensor_info->sensor_width = FIRST_SENSOR_WIDTH;
	} else {
		sensor_info->sensor_width = (uint16_t)temp_int;
	}

	ret = module_config_get_int("sensor_height", &temp_int);
	if (ret < 0) {
		sensor_info->sensor_height = FIRST_SENSOR_HEIGHT;
	} else {
		sensor_info->sensor_height = (uint16_t)temp_int;
	}

	ret = module_config_get_int("sensor_fps", &temp_int);
	if (ret < 0) {
		sensor_info->sensor_fps = 30;
	} else {
		sensor_info->sensor_fps = (uint8_t)temp_int;
	}

	return 0;
}

#if (SENSOR_NUM == 2)
int sensor2_info_init(sensor_info_t *sensor_info)
{
	int ret;
	int temp_int; // 使用这个变量去获取int型的数据，防止非int型变量引入地址时发生强转int型导致内存数据出错

	ret = module_config_get_string("sensor2_name", (char*)sensor_info->sensor_name);
	if (ret < 0) {
		strcpy((char*)sensor_info->sensor_name, FIRST_SENSOR_NAME);
	}

	ret = module_config_get_hex("sensor2_i2c_addr", &temp_int);
	if (ret < 0) {
		sensor_info->i2c_addr = FIRST_I2C_ADDR;
	} else {
		sensor_info->i2c_addr = (uint16_t)temp_int;
	}

	ret = module_config_get_int("sensor2_default_boot", &temp_int);
	if (ret < 0) {
		sensor_info->default_boot = FIRST_DEFAULT_BOOT;
	} else {
		sensor_info->default_boot = (uint8_t)temp_int;
	}

	ret = module_config_get_int("sensor2_width", &temp_int);
	if (ret < 0) {
		sensor_info->sensor_width = FIRST_SENSOR_WIDTH;
	} else {
		sensor_info->sensor_width = (uint16_t)temp_int;
	}

	ret = module_config_get_int("sensor2_height", &temp_int);
	if (ret < 0) {
		sensor_info->sensor_height = FIRST_SENSOR_HEIGHT;
	} else {
		sensor_info->sensor_height = (uint16_t)temp_int;
	}

	ret = module_config_get_int("sensor2_fps", &temp_int);
	if (ret < 0) {
		sensor_info->sensor_fps = 30;
	} else {
		sensor_info->sensor_fps = (uint8_t)temp_int;
	}

	return 0;
}
#endif

int encoder_info_init(encoder_info_t *encoder_info)
{
	int ret;
	int temp_int; // 使用这个变量去获取int型的数据，防止非int型变量引入地址时发生强转int型导致内存数据出错

	ret = module_config_get_int("rcmode", &temp_int);
	if (ret < 0) {
		encoder_info->rcmode = 1 ;	/* IMP_ENC_RC_MODE_CBR */
	} else {
		encoder_info->rcmode = (uint8_t)temp_int;
	}

	ret = module_config_get_int("gop", &temp_int);
	if (ret < 0) {
		encoder_info->gop = 25;
	} else {
		encoder_info->gop = (uint8_t)temp_int;
	}

	ret = module_config_get_int("bitrate", &temp_int);
	if (ret < 0) {
		encoder_info->bitrate = 4000;
	} else {
		encoder_info->bitrate = (uint32_t)temp_int;
	}

	ret = module_config_get_int("qp_value", &temp_int);
	if (ret < 0) {
		encoder_info->qp_value = 35;
	} else {
		encoder_info->qp_value = (uint8_t)temp_int;
	}

#ifdef T41
	ret = module_config_get_int("qp_reform", &temp_int);
	if (ret < 0) {
		encoder_info->qp_reform = 0;
	} else {
		encoder_info->qp_reform = (uint8_t)temp_int;
	}
	printf("INFO[%s]: encoder qp_reform enable(1) ro disable(0): %d\n",MODULE_TAG,encoder_info->qp_reform);
#endif
	return 0;
}
int imp_param_init(imp_attr_t* imp_attr)
{
	int ret;
	bcsh_info_t *bcsh_info = &imp_attr->bcsh_info;
	int temp_int; // 使用这个变量去获取int型的数据，防止非int型变量引入地址时发生强转int型导致内存数据出错

	/* sensor_info[0]: MAIN SENSOR */
	sensor_info_init(&imp_attr->sensor_info[0]);
#if (SENSOR_NUM == 2)
	sensor2_info_init(&imp_attr->sensor_info[1]);
#elif (VIDEO_NUM == 2)
	sensor_info_init(&imp_attr->sensor_info[1]);
#endif
	encoder_info_init(&imp_attr->encoder_info);

	ret = module_config_get_int("hvflip", &temp_int);
	if (ret < 0) {
		imp_attr->hvflip = 3;
	} else {
		imp_attr->hvflip = (uint8_t)temp_int;
	}

	ret = module_config_get_int("bcsh_en", &temp_int);
	if (ret < 0) {
		bcsh_info->bcsh_en = 0;
	} else {
		bcsh_info->bcsh_en = (uint8_t)temp_int;
	}

	ret = module_config_get_int("bcsh_low", &temp_int);
	if (ret < 0) {
		bcsh_info->bcsh_low = 15;
	} else {
		bcsh_info->bcsh_low = (uint8_t)temp_int;
	}

	ret = module_config_get_int("bcsh_high", &temp_int);
	if (ret < 0) {
		bcsh_info->bcsh_high = 235;
	} else {
		bcsh_info->bcsh_high = (uint8_t)temp_int;
	}

#if defined(T41) || defined(T23)
	ret = module_config_get_int("ivdc_mode", &temp_int);
	if (ret < 0) {
		imp_attr->ivdc_info.ivdc_mode = 0;
	} else {
		imp_attr->ivdc_info.ivdc_mode = (uint8_t)temp_int;
	}

	ret = module_config_get_int("ivdc_jpegv", &temp_int);
	if (ret < 0) {
		imp_attr->ivdc_info.ivdc_jpegv = 0;
	} else {
		imp_attr->ivdc_info.ivdc_jpegv = (uint8_t)temp_int;
	}
#endif

#ifdef OSD_ENABLE
	ret = module_config_get_int("osd_mode", &temp_int);
	if (ret < 0) {
		imp_attr->osd_mode = ISP_OSD;
	} else {
		imp_attr->osd_mode = (uint8_t)temp_int;
	}

	ret = module_config_get_int("osd_mem", &temp_int);
	if (ret < 0) {
		imp_attr->osd_mem = OSD_MEM;
	} else {
		imp_attr->osd_mem = (uint8_t)temp_int;
	}
#endif
	ret = module_config_get_int("wdr_en", &temp_int);
	if (ret < 0) {
		imp_attr->wdr_en = 0;
	} else {
		imp_attr->wdr_en = (uint8_t)temp_int;
	}

	return 0;
}
/* end of imp_info_init */

int af_param_init(af_param_t* af_param)
{
	int ret = 0;
	int temp_int; // 使用这个变量去获取int型的数据，防止非int型变量引入地址时发生强转int型导致内存数据出错

	ret = module_config_get_int("af_on_off", &temp_int);
	if (ret < 0) {
		af_param->af_en = AF_DISABLE;
	} else {
		af_param->af_en = (uint8_t)temp_int;
	}

	ret = module_config_get_int("motor_pixel", &temp_int);
	if (ret < 0) {
		af_param->motor_pixel = MOTOR_PIXEL;
	} else {
		af_param->motor_pixel = (uint16_t)temp_int;
	}

	ret = module_config_get_int("motor_step", &temp_int);
	if (ret < 0) {
		af_param->motor_step = MOTOR_STEP;
	} else {
		af_param->motor_step = (uint16_t)temp_int;
	}

	ret = module_config_get_int("motor_step_max", &temp_int);
	if (ret < 0) {
		af_param->motor_step_max = MOTOR_STEP_MAX;
	} else {
		af_param->motor_step_max = (uint16_t)temp_int;
	}

	ret = module_config_get_int("motor_step_min", &temp_int);
	if (ret < 0) {
		af_param->motor_step_min = MOTOR_STEP_MIN;
	} else {
		af_param->motor_step_min = (uint16_t)temp_int;
	}

	ret = module_config_get_int("motor_sleep_time", &temp_int);
	if (ret < 0) {
		af_param->motor_sleep_time = MOTOR_STEP_TIME;
	} else {
		af_param->motor_sleep_time = (uint32_t)temp_int;
	}

	ret = module_config_get_int("max_step", &temp_int);
	if (ret < 0) {
		af_param->max_step = MAX_STEP;
	} else {
		af_param->max_step = (uint16_t)temp_int;
	}

	ret = module_config_get_int("min_step", &temp_int);
	if (ret < 0) {
		af_param->min_step = MIN_STEP;
	} else {
		af_param->min_step = (uint16_t)temp_int;
	}

	ret = module_config_get_int("focus_trigger_value", &temp_int);
	if (ret < 0) {
		af_param->focus_trigger_value = FOCUS_TRIGGER_VALUE;
	} else {
		af_param->focus_trigger_value = (uint8_t)temp_int;
	}

	return 0;
}

#ifdef MODULE_FACEZOOM_ENABLE
int facezoom_param_init(facezoom_func_param_t* facezoom_param)
{
	int ret = 0;
	int temp_int; // 使用这个变量去获取int型的数据，防止非int型变量引入地址时发生强转int型导致内存数据出错

	ret = module_config_get_int("facezoom_en", &temp_int);
	if (ret < 0) {
		facezoom_param->facezoom_en = FACEZOOM_ENABLE;
	}else {
		facezoom_param->facezoom_en = (uint8_t)temp_int;
	}
	ret = module_config_get_int("facezoom_mode", &temp_int);
	if (ret < 0) {
		facezoom_param->facezoom_mode = MULTI_MODE;
	}else {
		facezoom_param->facezoom_mode = (uint8_t)temp_int;
	}
	return 0;
}
#endif

#ifdef MODULE_PIP_ENABLE
int pip_param_init(pip_func_param_t* pip_param)
{
	int ret = 0;
	int temp_int; // 使用这个变量去获取int型的数据，防止非int型变量引入地址时发生强转int型导致内存数据出错

	ret = module_config_get_int("pip_en", &temp_int);
	if (ret < 0) {
		pip_param->pip_en = PIP_ENABLE;
	}else {
		pip_param->pip_en = (uint8_t)temp_int;
	}
	ret = module_config_get_int("pip_mode", &temp_int);
	if (ret < 0) {
		pip_param->pip_mode = 0;
	}else {
		pip_param->pip_mode = (uint8_t)temp_int;
	}
	return 0;
}
#endif

int system_config_init(void *param)
{
	config_func_param_t *cfg_param = (config_func_param_t*)param;

	device_param_init(&cfg_param->dev_info);
	video_param_init(&cfg_param->video_info[0]);
#if (VIDEO_NUM == 2)
	video2_param_init(&cfg_param->video_info[1]);
#endif
	audio_param_init(&cfg_param->audio_info);
	imp_param_init(&cfg_param->imp_attr);
	af_param_init(&cfg_param->af_param);
#ifdef MODULE_FACEZOOM_ENABLE
	facezoom_param_init(&cfg_param->facezoom_param);
#endif
#ifdef MODULE_PIP_ENABLE
	pip_param_init(&cfg_param->pip_param);
#endif

	return 0;
}

