/*
 * sample-common.c
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 */

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <semaphore.h>

#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_framesource.h>
#include <imp/imp_encoder.h>
#include <imp/imp_isp.h>
#include <imp/imp_osd.h>
#include <imp/imp_audio.h>

#include <usbcamera.h>
#include "imp-common.h"
#include "global_config.h"
#include <imp_isposd.h>

#define TAG "IMP_COMMON"

static IMPSensorInfo sensor_info;
uint8_t ivdc_mode = 1;

typedef struct _imp_common_attr
{
	int sensor_width;
	int sensor_height;
	int low_power;
	int rcmode;
	int bitrate;
	int gop;
	int qp_value;
} imp_common_attr_t;

static imp_common_attr_t imp_ctx = {
	.sensor_width = 1920,
	.sensor_height = 1080,
	.rcmode = ENC_RC_MODE_CBR,
	.bitrate = 1000,
	.gop = SENSOR_FRAME_RATE_NUM_30,
	.qp_value = 80,
};

static uint32_t frame_format[VIDEO_NUM];

//#define SHOW_FRM_BITRATE
#ifdef SHOW_FRM_BITRATE
#define FRM_BIT_RATE_TIME 2
#define STREAM_TYPE_NUM 3
static int frmrate_sp[STREAM_TYPE_NUM] = {0};
static int statime_sp[STREAM_TYPE_NUM] = {0};
static int bitrate_sp[STREAM_TYPE_NUM] = {0};
#endif

struct chn_conf chn[FS_CHN_NUM] = {
	{
		.index = CH0_INDEX,
		.enable = CHN0_EN,
		.payloadType = PT_H264,

		.fs_chn_attr = {
			.pixFmt = PIX_FMT_NV12,
			.outFrmRateNum = SENSOR_FRAME_RATE_NUM_30,
			.outFrmRateDen = SENSOR_FRAME_RATE_DEN,
			.nrVBs = 2,
			.type = FS_PHY_CHANNEL,

			.crop.enable = 0,
			.crop.top = 0,
			.crop.left = 0,
			.crop.width = SENSOR_WIDTH_THIRD,
			.crop.height = SENSOR_HEIGHT_THIRD,

			.scaler.enable = 0,
			.scaler.outwidth = SENSOR_WIDTH_THIRD,
			.scaler.outheight = SENSOR_HEIGHT_THIRD,
			.picWidth = SENSOR_WIDTH_THIRD,
			.picHeight = SENSOR_HEIGHT_THIRD,
		},
		.framesource_chn = {DEV_ID_FS, CH0_INDEX, 0},
		.imp_encoder = {DEV_ID_ENC, CH0_INDEX, 0},
	},
	{
		.index = CH1_INDEX,
		/*.enable = 1,*/
		.enable = CHN1_EN,
		.fs_chn_attr = {
			.pixFmt = PIX_FMT_NV12,
			.outFrmRateNum = SENSOR_FRAME_RATE_NUM_30,
			.outFrmRateDen = SENSOR_FRAME_RATE_DEN,
			.nrVBs = 2,
			.type = FS_PHY_CHANNEL,

			.crop.enable = 0,
			.crop.top = 0,
			.crop.left = 0,
			.crop.width = SENSOR_WIDTH_SECOND,
			.crop.height = SENSOR_HEIGHT_SECOND,

			.scaler.enable = 1,
			.scaler.outwidth = SENSOR_WIDTH_SECOND,
			.scaler.outheight = SENSOR_HEIGHT_SECOND,

			.picWidth = SENSOR_WIDTH_SECOND,
			.picHeight = SENSOR_HEIGHT_SECOND,
		},
		.framesource_chn = {DEV_ID_FS, 1, 0},
		.imp_encoder = {DEV_ID_ENC, 1, 0},
	},
};

extern int IMP_OSD_SetPoolSize(int size);

#ifdef SHOW_FRM_BITRATE
int frame_bitrate_show(IMPEncoderStream *stream)
{
	int chnNum = 0;
	int i, len = 0;
	for (i = 0; i < stream->packCount; i++)
	{
		len += stream->pack[i].length;
	}
	bitrate_sp[chnNum] += len;
	frmrate_sp[chnNum]++;

	int64_t now = IMP_System_GetTimeStamp() / 1000;
	if (((int)(now - statime_sp[chnNum]) / 1000) >= FRM_BIT_RATE_TIME)
	{
		double fps = (double)frmrate_sp[chnNum] / ((double)(now - statime_sp[chnNum]) / 1000);
		double kbr = (double)bitrate_sp[chnNum] * 8 / (double)(now - statime_sp[chnNum]);

		printf("streamNum[%d]:FPS: %0.2f,Bitrate: %0.2f(kbps)\n", chnNum, fps, kbr);
		//fflush(stdout);

		frmrate_sp[chnNum] = 0;
		bitrate_sp[chnNum] = 0;
		statime_sp[chnNum] = now;
	}
	return 0;
}
#endif


static int imp_isp_tuning_init(void)
{
	int ret;

	/*TODO: */
	uvc_pu_powerlinefreq_set(SENSOR_MAIN, IMPISP_ANTIFLICKER_60HZ);

	IMPISPRunningMode dn = IMPISP_RUNNING_MODE_DAY;
	ret = IMP_ISP_Tuning_SetISPRunningMode(dn);
	if (ret < 0){
		IMP_LOG_ERR(TAG, "failed to set video(%d) running mode\n");
		return -1;
	}

	/*Step4: set sensor hvflip */
	imp_attr_t *imp_attr = &g_func_param.imp_attr;
	int hvflip = imp_attr->hvflip;
	printf("----hvflip = %d----\n", hvflip);

	if ((hvflip > IMPISP_FLIP_NORMAL_MODE) && (hvflip < IMPISP_FLIP_MODE_BUTT)) {

		IMPISPHVFLIP testhv = hvflip;
		ret = IMP_ISP_Tuning_SetHVFLIP(testhv);
		if (ret < 0){
			IMP_LOG_ERR(TAG, "failed to set HV Filp mode\n");
			return -1;
		}
	}

	return 0;

}

int sample_system_init(void *param)
{
	int ret = 0;

	imp_attr_t *imp_param = (imp_attr_t *)param;
	if (!imp_param)
	{
		printf("ERROR(%s): imp_param is NULL \n", TAG);
		return -1;
	}

	sensor_info_t *sp = &imp_param->sensor_info[SENSOR_MAIN];
	imp_ctx.bitrate = imp_param->encoder_info.bitrate;
	imp_ctx.gop = imp_param->encoder_info.gop;
	imp_ctx.qp_value = imp_param->encoder_info.qp_value;
	imp_ctx.rcmode = imp_param->encoder_info.rcmode;
	imp_ctx.sensor_width = sp->sensor_width;
	imp_ctx.sensor_height = sp->sensor_height;

	ivdc_mode = imp_param->ivdc_info.ivdc_mode;

#ifdef OSD_ENABLE
	if ( IPU_OSD == imp_param->osd_mode){
		IMP_OSD_SetPoolSize(512 * 1024 * imp_param->osd_mem);
	} else if ( ISP_OSD == imp_param->osd_mode){
		IMP_ISP_Tuning_SetOsdPoolSize( 512 * 1024 * imp_param->osd_mem );
	} else if (IPU_ISP_OSD == imp_param->osd_mode){
		IMP_OSD_SetPoolSize(512 * 1024 * imp_param->osd_mem);
		IMP_ISP_Tuning_SetOsdPoolSize( 512 * 1024 * imp_param->osd_mem );
	} else {
		printf("INFO[%s] --> disable osd <--\n",TAG);
	}
#endif

	memset(&sensor_info, 0, sizeof(IMPSensorInfo));
	memcpy(sensor_info.name, (char *)sp->sensor_name, sizeof(sp->sensor_name));
	sensor_info.cbus_type = TX_SENSOR_CONTROL_INTERFACE_I2C;
	memcpy(sensor_info.i2c.type, (char *)sp->sensor_name, sizeof(sp->sensor_name));
	sensor_info.i2c.addr = sp->i2c_addr;

	IMP_LOG_DBG(TAG, "sample_system_init start\n");

	/*Step1: Open Sensor */
	ret = IMP_ISP_Open();
	if (ret)
	{
		IMP_LOG_ERR(TAG, "failed to open ISP\n");
		return -1;
	}

	/*Step2: AddSernsor */
	ret = IMP_ISP_AddSensor(&sensor_info);
	if (ret < 0)
	{
		IMP_LOG_ERR(TAG, "failed to AddSensor\n");
		return -1;
	}

	/*Step3: EnableSensor */
	ret = IMP_ISP_EnableSensor();
	if (ret < 0)
	{
		IMP_LOG_ERR(TAG, "failed to EnableSensor\n");
		return -1;
	}

	/*Step4: System Init */
	ret = IMP_System_Init();
	if (ret < 0)
	{
		IMP_LOG_ERR(TAG, "IMP_System_Init failed\n");
		return -1;
	}

	ret = IMP_ISP_EnableTuning();
	if (ret < 0)
	{
		IMP_LOG_ERR(TAG, "IMP_ISP_EnableTuning failed\n");
		return -1;
	}

	imp_isp_tuning_init();

	IMP_LOG_DBG(TAG, "ImpSystemInit success\n");

	return 0;
}

int sample_system_exit()
{
	int ret = 0;

	IMP_LOG_DBG(TAG, "sample_system_exit start\n");

	IMP_System_Exit();

	ret = IMP_ISP_DisableTuning();
	if (ret < 0)
	{
		IMP_LOG_ERR(TAG, "IMP_ISP_DisableTuning failed\n");
		return -1;
	}

	ret = IMP_ISP_DisableSensor();
	if (ret < 0)
	{
		IMP_LOG_ERR(TAG, "failed to EnableSensor\n");
		return -1;
	}

	ret = IMP_ISP_DelSensor(&sensor_info);
	if (ret < 0)
	{
		IMP_LOG_ERR(TAG, "failed to AddSensor\n");
		return -1;
	}

	if (IMP_ISP_Close())
	{
		IMP_LOG_ERR(TAG, "failed to open ISP\n");
		return -1;
	}

	IMP_LOG_DBG(TAG, " sample_system_exit success\n");

	return 0;
}

int sample_sensor_setfps(int sensor, int fps_num, int fps_den)
{
	int ret = 0;

	ret = IMP_ISP_Tuning_SetSensorFPS(fps_num, fps_den);
	if ( ret < 0 ) {
		printf(" ** uvc set sensor fps failed **\n");
		return ret;
	}

	return ret;
}

extern uint32_t v4l2_buff_size[VIDEO_NUM]; /* Actual v4l2 buff size used*/
static int get_stream(int video_index, char *buf, IMPEncoderStream *stream)
{
	int i, len = 0;
	int nr_pack = stream->packCount;
	int len_tmp = 0;

	for( i = 0; i < nr_pack; i++) {
		len_tmp += stream->pack[i].length;
		if(len_tmp > v4l2_buff_size[video_index]){
			printf("ERROR[%s]:stream size %d is lager than v4l2_buffer_size[%d] %d\n", TAG, len_tmp, video_index, v4l2_buff_size[video_index]);
			return -1;
		}
		memcpy(buf + len, (void*)stream->pack[i].virAddr, stream->pack[i].length);
		len += stream->pack[i].length;
	}
	return len;
}

int sample_get_h264_snap(int chn_num, char *img_buf)
{
	int chnNum, ret, len;
	chnNum = chn[chn_num].index;
	int video_index = ((chn_num == UVC_VIDEO_CH) ? 0 : 1);

	int frame_num;
	IMPFSChnAttr *imp_chn_attr_tmp;
	imp_chn_attr_tmp = &chn[chn_num].fs_chn_attr;

	frame_num = (imp_chn_attr_tmp->outFrmRateNum * 2 / imp_chn_attr_tmp->outFrmRateDen) * 2;

	ret = IMP_Encoder_PollingStream(chnNum, 1000);
	if (ret < 0)
	{
		IMP_LOG_ERR(TAG, "IMP_Encoder_PollingStream(%d) timeout\n", chnNum);
		return -1;
	}

	IMPEncoderStream stream;
	/* Get H264 or H265 Stream */
	ret = IMP_Encoder_GetStream(chnNum, &stream, 1);

	if (ret < 0)
	{
		IMP_LOG_ERR(TAG, "IMP_Encoder_GetStream(%d) failed\n", chnNum);
		return -1;
	}

	len = get_stream(video_index, img_buf, &stream);
	if(len < 0){
		IMP_Encoder_ReleaseStream(chnNum, &stream);
		return -1;
	}

#ifdef SHOW_FRM_BITRATE
	frame_bitrate_show(&stream);
#endif

	IMP_Encoder_ReleaseStream(chnNum, &stream);

	/*stream.seq = 120 is I frame*/
	if ((frame_num - 1) == stream.seq) {
		ret = IMP_Encoder_PollingStream(chnNum, 1000);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_Encoder_PollingStream(%d) timeout\n", chnNum);
			return -1;
		}

		IMPEncoderStream stream;
		/* Get H264 or H265 Stream */
		ret = IMP_Encoder_GetStream(chnNum, &stream, 1);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_Encoder_GetStream(%d) failed\n", chnNum);
			return -1;
		}

		len = get_stream(video_index, img_buf, &stream);
		if(len < 0){
			IMP_Encoder_ReleaseStream(chnNum, &stream);
			return -1;
		}
		IMP_Encoder_ReleaseStream(chnNum, &stream);
	}

	return len;
}

int sample_get_jpeg_snap(int chn_num, char *img_buf)
{
	int i = chn_num, ret ,len = 0;
	int chn_i = chn[i].index + ENCODER_MJPEG_CHANNEL_INDEX_PLUS;

	// int chnNum;
	// chnNum = chn[chn_num].index;
	int video_index = ((chn_num == UVC_VIDEO_CH) ? 0 : 1);

	if (chn[i].enable)
	{
		/* Polling JPEG Snap, set timeout as 1000msec */
		ret = IMP_Encoder_PollingStream(chn_i, 10000);
		if (ret < 0)
		{
			IMP_LOG_ERR(TAG, "Polling stream timeout\n");
			return 0;
		}

		IMPEncoderStream stream;
		/* Get JPEG Snap */
		ret = IMP_Encoder_GetStream(chn_i, &stream, 1);
		if (ret < 0)
		{
			IMP_LOG_ERR(TAG, "IMP_Encoder_GetStream() failed\n");
			return -1;
		}

		len = get_stream(video_index, img_buf, &stream);
		if(len < 0){
			IMP_Encoder_ReleaseStream(chn_i, &stream);
			return -1;
		}

		IMP_Encoder_ReleaseStream(chn_i, &stream);

#ifdef SHOW_FRM_BITRATE
		frame_bitrate_show(&stream);
#endif

	}
	return len;
}

/*convert NV12 to YUYV422*/
static void nv12toyuy2(char *image_in, char *image_out, int width, int height)
{
	int i, j;
	char *uv = image_in + width * height;
	for (j = 0; j < height; j++)
	{
		for (i = 0; i < width / 2; i++)
		{
			image_out[0] = image_in[2 * i];      //y
			image_out[1] = uv[2 * i];      //u
			image_out[2] = image_in[2 * i + 1]; //y
			image_out[3] = uv[2 * i + 1];      //v
			image_out += 4;
		}
		image_in += width;
		if (j & 1)
		{
			uv += width;
		}
	}
	return;
}

int sample_get_yuv_snap(int chn_num, char *img_buf)
{
	int ret, len = 0;
	IMPFrameInfo *frame_bak;
	IMPFSChnAttr *imp_chn_attr_tmp;
	int video_index = ((chn_num == UVC_VIDEO_CH) ? 0 : 1);

	if(ivdc_mode) {
		chn_num = CH1_INDEX;
	}

	imp_chn_attr_tmp = &chn[chn_num].fs_chn_attr;
	ret = IMP_FrameSource_GetFrame(chn_num, &frame_bak);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "%s(%d):IMP_FrameSource_GetFrame %d failed\n", __func__, __LINE__, chn_num);
		return -1;
	}

	if( frame_format[video_index] == V4L2_PIX_FMT_YUYV) {
		len = imp_chn_attr_tmp->picWidth *imp_chn_attr_tmp->picHeight *2;
		nv12toyuy2((char *)frame_bak->virAddr, img_buf, imp_chn_attr_tmp->picWidth, imp_chn_attr_tmp->picHeight);
	} else if(frame_format[video_index] == V4L2_PIX_FMT_NV12) {
		len = frame_bak->size;
		memcpy(img_buf, (char*)frame_bak->virAddr, frame_bak->size);
	}

	//len = frame_bak->size;
	//memcpy(img_buf, frame_bak->virAddr, len);
	IMP_FrameSource_ReleaseFrame(chn_num, frame_bak);
	if (ret < 0)
	{
		IMP_LOG_ERR(TAG, "%s(%d):IMP_FrameSource_ReleaseFrame failed\n", __func__, __LINE__);
		return -1;
	}
	return len;
}


static int sample_jpeg_encoder_init(int chn_num)
{
	int i = chn_num, ret;
	int chn_i = chn[i].index + ENCODER_MJPEG_CHANNEL_INDEX_PLUS;

	IMPEncoderCHNAttr channel_attr;
	IMPEncoderAttr *encAttr;

	IMPFSChnAttr *imp_chn_attr_tmp;

	if (chn[i].enable)
	{
		ret = IMP_Encoder_CreateGroup(chn[i].index);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_Encoder_CreateGroup(%d) error !\n", i);
			return -1;
		}

		imp_chn_attr_tmp = &chn[i].fs_chn_attr;

		memset(&channel_attr, 0, sizeof(IMPEncoderCHNAttr));
		encAttr = &channel_attr.encAttr;
		encAttr->enType = PT_JPEG;
		encAttr->bufSize = 0;
		encAttr->profile = 0;
		encAttr->picWidth = imp_chn_attr_tmp->picWidth;
		encAttr->picHeight = imp_chn_attr_tmp->picHeight;

		if(ivdc_mode == 1){
			if(0 == chn_num)
				channel_attr.bEnableIvdc = true;
		}

		/* Create Channel */
		ret = IMP_Encoder_CreateChn(chn_i, &channel_attr);
		if (ret < 0)
		{
			IMP_LOG_ERR(TAG, "IMP_Encoder_CreateChn(%d) error: %d\n",
				    chn[i].index, ret);
			return -1;
		}

		/* Resigter Channel */
		ret = IMP_Encoder_RegisterChn(i, chn_i);
		if (ret < 0)
		{
			IMP_LOG_ERR(TAG, "IMP_Encoder_RegisterChn(0, %d) error: %d\n",
				    chn[i].index, ret);
			return -1;
		}
	}

	return 0;
}

static int sample_jpeg_encoder_exit(int chn_num)
{
	int ret = 0, i = 0, chnNum = 0;

	IMPEncoderCHNStat chn_stat;
	memset(&chn_stat, 0, sizeof(IMPEncoderCHNStat));

	i = chn_num;
	if (chn[i].enable)
	{
		chnNum = ENCODER_MJPEG_CHANNEL_INDEX_PLUS + chn[i].index;
		ret = IMP_Encoder_Query(chnNum, &chn_stat);
		if (ret < 0)
		{
			IMP_LOG_ERR(TAG, "IMP_Encoder_Query(%d) error: %d\n", chnNum, ret);
			return -1;
		}

		if (chn_stat.registered)
		{
			ret = IMP_Encoder_UnRegisterChn(chnNum);
			if (ret < 0)
			{
				IMP_LOG_ERR(TAG, "IMP_Encoder_UnRegisterChn(%d) error: %d\n", chnNum, ret);
				return -1;
			}

			ret = IMP_Encoder_DestroyChn(chnNum);
			if (ret < 0)
			{
				IMP_LOG_ERR(TAG, "IMP_Encoder_DestroyChn(%d) error: %d\n", chnNum, ret);
				return -1;
			}
		}

		ret = IMP_Encoder_DestroyGroup(chn[i].index);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_Encoder_DestroyGroup(%d) error !\n", i);
			return -1;
		}

	}

	return 0;
}

static int sample_h264_encoder_init(int chn_index)
{
	int i, ret, chnNum = 0;
	int video_index = ((chn_index == UVC_VIDEO_CH) ? 0 : 1);

	i = chn_index;
	chnNum = chn[i].index;

	IMPFSChnAttr *imp_chn_attr_tmp;
	IMPEncoderCHNAttr channel_attr;//最后设置的通道属性结构体
	IMPEncoderAttr *enc_attr;//通道encoder参数
	IMPEncoderRcAttr *rc_attr;//通道rcmode参数

	imp_chn_attr_tmp = &chn[i].fs_chn_attr;

	memset(&channel_attr, 0, sizeof(IMPEncoderCHNAttr));

	enc_attr = &channel_attr.encAttr;//编码参数
	rc_attr = &channel_attr.rcAttr;//RC的参数

	if( frame_format[video_index] == V4L2_PIX_FMT_H264)
		chn[i].payloadType = PT_H264;
	else if( frame_format[video_index] == V4L2_PIX_FMT_H265)
		chn[i].payloadType = PT_H265;

	ret = IMP_Encoder_CreateGroup(chnNum);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_Encoder_CreateGroup(%d) error !\n", i);
		return -1;
	}

	/***********************************Enc_Attr 赋值操作*****************************************/
	enc_attr->enType = chn[i].payloadType;

	if ((imp_chn_attr_tmp->picHeight > 1920) || (imp_chn_attr_tmp->picHeight == 1920)) {
		enc_attr->bufSize = imp_chn_attr_tmp->picWidth*imp_chn_attr_tmp->picHeight*3/10;
	} else if ((imp_chn_attr_tmp->picHeight > 1520) || (imp_chn_attr_tmp->picHeight == 1520)) {
		enc_attr->bufSize = imp_chn_attr_tmp->picWidth*imp_chn_attr_tmp->picHeight*3/8;
	} else if ((imp_chn_attr_tmp->picHeight > 1080) || (imp_chn_attr_tmp->picHeight == 1080)) {
		enc_attr->bufSize = imp_chn_attr_tmp->picWidth*imp_chn_attr_tmp->picHeight/2;
	} else {
		enc_attr->bufSize = imp_chn_attr_tmp->picWidth*imp_chn_attr_tmp->picHeight*3/4;
	}

	enc_attr->profile = 1;
	enc_attr->picWidth = imp_chn_attr_tmp->picWidth;
	enc_attr->picHeight = imp_chn_attr_tmp->picHeight;

	/***********************************RC_Attr 赋值操作*****************************************/
	if (chn[i].payloadType == PT_H264)
	{
		enc_attr->profile = -1;
		rc_attr->outFrmRate.frmRateNum = imp_chn_attr_tmp->outFrmRateNum;
		rc_attr->outFrmRate.frmRateDen = imp_chn_attr_tmp->outFrmRateDen;
		rc_attr->maxGop = 2 * rc_attr->outFrmRate.frmRateNum / rc_attr->outFrmRate.frmRateDen;
		if (imp_ctx.rcmode == ENC_RC_MODE_CBR) {
			rc_attr->attrRcMode.rcMode = ENC_RC_MODE_CBR;
			rc_attr->attrRcMode.attrH264Cbr.outBitRate = (uint64_t)imp_ctx.bitrate * (imp_chn_attr_tmp->picWidth * imp_chn_attr_tmp->picHeight) / (1280 * 720);
			rc_attr->attrRcMode.attrH264Cbr.maxQp = 45;
			rc_attr->attrRcMode.attrH264Cbr.minQp = 15;
			rc_attr->attrRcMode.attrH264Cbr.iBiasLvl = 0;
			rc_attr->attrRcMode.attrH264Cbr.frmQPStep = 3;
			rc_attr->attrRcMode.attrH264Cbr.gopQPStep = 15;
			rc_attr->attrRcMode.attrH264Cbr.adaptiveMode = false;
			rc_attr->attrRcMode.attrH264Cbr.gopRelation = false;

			rc_attr->attrHSkip.hSkipAttr.skipType = IMP_Encoder_STYPE_N1X;
			rc_attr->attrHSkip.hSkipAttr.m = 0;
			rc_attr->attrHSkip.hSkipAttr.n = 0;
			rc_attr->attrHSkip.hSkipAttr.maxSameSceneCnt = 0;
			rc_attr->attrHSkip.hSkipAttr.bEnableScenecut = 0;
			rc_attr->attrHSkip.hSkipAttr.bBlackEnhance = 0;
			rc_attr->attrHSkip.maxHSkipType = IMP_Encoder_STYPE_N1X;
		} else if (imp_ctx.rcmode == ENC_RC_MODE_VBR) {
			rc_attr->attrRcMode.rcMode = ENC_RC_MODE_VBR;
			rc_attr->attrRcMode.attrH264Vbr.maxQp = 45;
			rc_attr->attrRcMode.attrH264Vbr.minQp = 15;
			rc_attr->attrRcMode.attrH264Vbr.staticTime = 2;
			rc_attr->attrRcMode.attrH264Vbr.maxBitRate = (uint64_t)imp_ctx.bitrate * (imp_chn_attr_tmp->picWidth * imp_chn_attr_tmp->picHeight) / (1280 * 720);
			rc_attr->attrRcMode.attrH264Vbr.iBiasLvl = 0;
			rc_attr->attrRcMode.attrH264Vbr.changePos = 80;
			rc_attr->attrRcMode.attrH264Vbr.qualityLvl = 2;
			rc_attr->attrRcMode.attrH264Vbr.frmQPStep = 3;
			rc_attr->attrRcMode.attrH264Vbr.gopQPStep = 15;
			rc_attr->attrRcMode.attrH264Vbr.gopRelation = false;

			rc_attr->attrHSkip.hSkipAttr.skipType = IMP_Encoder_STYPE_N1X;
			rc_attr->attrHSkip.hSkipAttr.m = 0;
			rc_attr->attrHSkip.hSkipAttr.n = 0;
			rc_attr->attrHSkip.hSkipAttr.maxSameSceneCnt = 0;
			rc_attr->attrHSkip.hSkipAttr.bEnableScenecut = 0;
			rc_attr->attrHSkip.hSkipAttr.bBlackEnhance = 0;
			rc_attr->attrHSkip.maxHSkipType = IMP_Encoder_STYPE_N1X;
		} else if (imp_ctx.rcmode == ENC_RC_MODE_SMART) {
			rc_attr->attrRcMode.rcMode = ENC_RC_MODE_SMART;
			rc_attr->attrRcMode.attrH264Smart.maxQp = 45;
			rc_attr->attrRcMode.attrH264Smart.minQp = 15;
			rc_attr->attrRcMode.attrH264Smart.staticTime = 2;
			rc_attr->attrRcMode.attrH264Smart.maxBitRate = (uint64_t)imp_ctx.bitrate * (imp_chn_attr_tmp->picWidth * imp_chn_attr_tmp->picHeight) / (1280 * 720);
			rc_attr->attrRcMode.attrH264Smart.iBiasLvl = 0;
			rc_attr->attrRcMode.attrH264Smart.changePos = 80;
			rc_attr->attrRcMode.attrH264Smart.qualityLvl = 2;
			rc_attr->attrRcMode.attrH264Smart.frmQPStep = 3;
			rc_attr->attrRcMode.attrH264Smart.gopQPStep = 15;
			rc_attr->attrRcMode.attrH264Smart.gopRelation = false;

			rc_attr->attrHSkip.hSkipAttr.skipType = IMP_Encoder_STYPE_N1X;
			rc_attr->attrHSkip.hSkipAttr.m = rc_attr->maxGop - 1;
			rc_attr->attrHSkip.hSkipAttr.n = 1;
			rc_attr->attrHSkip.hSkipAttr.maxSameSceneCnt = 6;
			rc_attr->attrHSkip.hSkipAttr.bEnableScenecut = 0;
			rc_attr->attrHSkip.hSkipAttr.bBlackEnhance = 0;
			rc_attr->attrHSkip.maxHSkipType = IMP_Encoder_STYPE_N1X;
		} else { /* fixQp */
			rc_attr->attrRcMode.rcMode = ENC_RC_MODE_FIXQP;
			rc_attr->attrRcMode.attrH264FixQp.qp = 35;

			rc_attr->attrHSkip.hSkipAttr.skipType = IMP_Encoder_STYPE_N1X;
			rc_attr->attrHSkip.hSkipAttr.m = 0;
			rc_attr->attrHSkip.hSkipAttr.n = 0;
			rc_attr->attrHSkip.hSkipAttr.maxSameSceneCnt = 0;
			rc_attr->attrHSkip.hSkipAttr.bEnableScenecut = 0;
			rc_attr->attrHSkip.hSkipAttr.bBlackEnhance = 0;
			rc_attr->attrHSkip.maxHSkipType = IMP_Encoder_STYPE_N1X;
		}
	} else { //PT_H265
		rc_attr->outFrmRate.frmRateNum = imp_chn_attr_tmp->outFrmRateNum;
		rc_attr->outFrmRate.frmRateDen = imp_chn_attr_tmp->outFrmRateDen;
		rc_attr->maxGop = 2 * rc_attr->outFrmRate.frmRateNum / rc_attr->outFrmRate.frmRateDen;
		if (imp_ctx.rcmode == ENC_RC_MODE_CBR) {
			rc_attr->attrRcMode.attrH265Cbr.maxQp = 45;
			rc_attr->attrRcMode.attrH265Cbr.minQp = 15;
			rc_attr->attrRcMode.attrH265Cbr.staticTime = 2;
			rc_attr->attrRcMode.attrH265Cbr.outBitRate = (uint64_t)imp_ctx.bitrate * (imp_chn_attr_tmp->picWidth * imp_chn_attr_tmp->picHeight) / (1280 * 720);
			rc_attr->attrRcMode.attrH265Cbr.iBiasLvl = 0;
			rc_attr->attrRcMode.attrH265Cbr.frmQPStep = 3;
			rc_attr->attrRcMode.attrH265Cbr.gopQPStep = 15;
			rc_attr->attrRcMode.attrH265Cbr.flucLvl = 2;

			rc_attr->attrHSkip.hSkipAttr.skipType = IMP_Encoder_STYPE_N1X;
			rc_attr->attrHSkip.hSkipAttr.m = 0;
			rc_attr->attrHSkip.hSkipAttr.n = 0;
			rc_attr->attrHSkip.hSkipAttr.maxSameSceneCnt = 0;
			rc_attr->attrHSkip.hSkipAttr.bEnableScenecut = 0;
			rc_attr->attrHSkip.hSkipAttr.bBlackEnhance = 0;
			rc_attr->attrHSkip.maxHSkipType = IMP_Encoder_STYPE_N1X;
		} else if (imp_ctx.rcmode == ENC_RC_MODE_VBR) {
			rc_attr->attrRcMode.rcMode = ENC_RC_MODE_VBR;
			rc_attr->attrRcMode.attrH265Vbr.maxQp = 45;
			rc_attr->attrRcMode.attrH265Vbr.minQp = 15;
			rc_attr->attrRcMode.attrH265Vbr.staticTime = 2;
			rc_attr->attrRcMode.attrH265Vbr.maxBitRate = (uint64_t)imp_ctx.bitrate * (imp_chn_attr_tmp->picWidth * imp_chn_attr_tmp->picHeight) / (1280 * 720);
			rc_attr->attrRcMode.attrH265Vbr.iBiasLvl = 0;
			rc_attr->attrRcMode.attrH265Vbr.changePos = 80;
			rc_attr->attrRcMode.attrH265Vbr.qualityLvl = 2;
			rc_attr->attrRcMode.attrH265Vbr.frmQPStep = 3;
			rc_attr->attrRcMode.attrH265Vbr.gopQPStep = 15;
			rc_attr->attrRcMode.attrH265Vbr.flucLvl = 2;

			rc_attr->attrHSkip.hSkipAttr.skipType = IMP_Encoder_STYPE_N1X;
			rc_attr->attrHSkip.hSkipAttr.m = 0;
			rc_attr->attrHSkip.hSkipAttr.n = 0;
			rc_attr->attrHSkip.hSkipAttr.maxSameSceneCnt = 0;
			rc_attr->attrHSkip.hSkipAttr.bEnableScenecut = 0;
			rc_attr->attrHSkip.hSkipAttr.bBlackEnhance = 0;
			rc_attr->attrHSkip.maxHSkipType = IMP_Encoder_STYPE_N1X;
		} else if (imp_ctx.rcmode == ENC_RC_MODE_SMART) {
			rc_attr->attrRcMode.rcMode = ENC_RC_MODE_SMART;
			rc_attr->attrRcMode.attrH265Smart.maxQp = 45;
			rc_attr->attrRcMode.attrH265Smart.minQp = 15;
			rc_attr->attrRcMode.attrH265Smart.staticTime = 2;
			rc_attr->attrRcMode.attrH265Smart.maxBitRate = (uint64_t)imp_ctx.bitrate * (imp_chn_attr_tmp->picWidth * imp_chn_attr_tmp->picHeight) / (1280 * 720);
			rc_attr->attrRcMode.attrH265Smart.iBiasLvl = 0;
			rc_attr->attrRcMode.attrH265Smart.changePos = 80;
			rc_attr->attrRcMode.attrH265Smart.qualityLvl = 2;
			rc_attr->attrRcMode.attrH265Smart.frmQPStep = 3;
			rc_attr->attrRcMode.attrH265Smart.gopQPStep = 15;
			rc_attr->attrRcMode.attrH265Smart.flucLvl = 2;

			rc_attr->attrHSkip.hSkipAttr.skipType = IMP_Encoder_STYPE_N1X;
			rc_attr->attrHSkip.hSkipAttr.m = rc_attr->maxGop - 1;
			rc_attr->attrHSkip.hSkipAttr.n = 1;
			rc_attr->attrHSkip.hSkipAttr.maxSameSceneCnt = 6;
			rc_attr->attrHSkip.hSkipAttr.bEnableScenecut = 0;
			rc_attr->attrHSkip.hSkipAttr.bBlackEnhance = 0;
			rc_attr->attrHSkip.maxHSkipType = IMP_Encoder_STYPE_N1X;
		} else { /* fixQp */
			rc_attr->attrRcMode.rcMode = ENC_RC_MODE_FIXQP;
			rc_attr->attrRcMode.attrH265FixQp.qp = 35;

			rc_attr->attrHSkip.hSkipAttr.skipType = IMP_Encoder_STYPE_N1X;
			rc_attr->attrHSkip.hSkipAttr.m = 0;
			rc_attr->attrHSkip.hSkipAttr.n = 0;
			rc_attr->attrHSkip.hSkipAttr.maxSameSceneCnt = 0;
			rc_attr->attrHSkip.hSkipAttr.bEnableScenecut = 0;
			rc_attr->attrHSkip.hSkipAttr.bBlackEnhance = 0;
			rc_attr->attrHSkip.maxHSkipType = IMP_Encoder_STYPE_N1X;
		}
	}

	if(ivdc_mode == 1) {
		if (0 == chnNum) channel_attr.bEnableIvdc = true;
	}

	ret = IMP_Encoder_CreateChn(chnNum, &channel_attr);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_Encoder_CreateChn(%d) error !\n", chnNum);
		return -1;
	}

	ret = IMP_Encoder_RegisterChn(chn[i].index, chnNum);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_Encoder_RegisterChn(%d, %d) error: %d\n", chn[i].index, chnNum, ret);
		return -1;
	}

	return 0;
}

static int sample_h264_encoder_exit(int chn_num)
{
	int ret = 0, i = 0, chnNum = 0;

	IMPEncoderCHNStat chn_stat;
	memset(&chn_stat, 0, sizeof(IMPEncoderCHNStat));

	i = chn_num;
	if (chn[i].enable)
	{
		chnNum = chn[i].index;
		ret = IMP_Encoder_Query(chnNum, &chn_stat);
		if (ret < 0)
		{
			IMP_LOG_ERR(TAG, "IMP_Encoder_Query(%d) error: %d\n", chnNum, ret);
			return -1;
		}

		if (chn_stat.registered)
		{
			ret = IMP_Encoder_UnRegisterChn(chnNum);
			if (ret < 0)
			{
				IMP_LOG_ERR(TAG, "IMP_Encoder_UnRegisterChn(%d) error: %d\n", chnNum, ret);
				return -1;
			}

			ret = IMP_Encoder_DestroyChn(chnNum);
			if (ret < 0)
			{
				IMP_LOG_ERR(TAG, "IMP_Encoder_DestroyChn(%d) error: %d\n", chnNum, ret);
				return -1;
			}
		}

		ret = IMP_Encoder_DestroyGroup(chn[i].index);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_Encoder_DestroyGroup(%d) error !\n", i);
			return -1;
		}

	}

	return 0;
}

static void imp_isp_scaler(int video_index, int width, int height)
{
	IMPFSChnAttr *imp_chn_attr_tmp = NULL;
	int chn_index = ((video_index < 1) ? UVC_VIDEO_CH : UVC2_VIDEO_CH);

	imp_chn_attr_tmp = &chn[chn_index].fs_chn_attr;

	/*Step1 frame info */
	imp_attr_t *imp_attr = &g_func_param.imp_attr;
	int sensor_width = imp_attr->sensor_info[video_index].sensor_width;
	int sensor_height = imp_attr->sensor_info[video_index].sensor_height;
	int sensor_fps = imp_attr->sensor_info[video_index].sensor_fps;

	imp_chn_attr_tmp->outFrmRateNum = sensor_fps;
	imp_chn_attr_tmp->picWidth = width;
	imp_chn_attr_tmp->picHeight = height;
	imp_chn_attr_tmp->nrVBs = 2;

	/* PS : resolution must be standard 16:9 or 4:3 */
	/* T21/T31/T40 : scaler, then crop */
	if (sensor_width == sensor_height * 4 / 3) {
		/* sensor setting - width : height = 4 : 3 */
		/*zoom need to open the scaler*/
		imp_chn_attr_tmp->scaler.enable    = 1;
		imp_chn_attr_tmp->scaler.outwidth  = width;
		imp_chn_attr_tmp->scaler.outheight = width * 3 / 4;

		/* set crop attr */
		if (width == (height * 16 / 9)) {
			imp_chn_attr_tmp->crop.enable = 1;
			imp_chn_attr_tmp->crop.left   = 0;
			/*imp_chn_attr_tmp->crop.top    = height / 6;*/
			imp_chn_attr_tmp->crop.top    = (imp_chn_attr_tmp->scaler.outheight - height) / 2;
			imp_chn_attr_tmp->crop.width  = width;
			imp_chn_attr_tmp->crop.height = height;
		} else {
			imp_chn_attr_tmp->crop.enable = 0;
		}
	} else {
		/* sensor setting - width : height = 16 : 9 */
		/*zoom need to open the scaler*/
		imp_chn_attr_tmp->scaler.enable    = 1;
		imp_chn_attr_tmp->scaler.outwidth  = height * 16 / 9;
		imp_chn_attr_tmp->scaler.outheight = height;

		/* set crop attr */
		if (width == (height * 4 / 3)) {
			imp_chn_attr_tmp->crop.enable = 1;
			/*imp_chn_attr_tmp->crop.left   = width / 6;*/
			imp_chn_attr_tmp->crop.left   = (imp_chn_attr_tmp->scaler.outwidth - width) / 2;
			imp_chn_attr_tmp->crop.top    = 0;
			imp_chn_attr_tmp->crop.width  = width;
			imp_chn_attr_tmp->crop.height = height;
		} else {
			imp_chn_attr_tmp->crop.enable = 0;
		}
	}
	if(width == sensor_width && height== sensor_height)
	{
		imp_chn_attr_tmp->scaler.enable = 0;
	}
	if (imp_chn_attr_tmp->scaler.enable) {
		while(imp_chn_attr_tmp->scaler.outwidth % 8)
			imp_chn_attr_tmp->scaler.outwidth--;

		/*if (imp_chn_attr_tmp->scaler.outwidth % 8)
		  imp_chn_attr_tmp->scaler.outwidth--;*/
	}

	if(imp_chn_attr_tmp->crop.enable){
		imp_chn_attr_tmp->crop.left   = (imp_chn_attr_tmp->scaler.outwidth - width) / 2;
	}

	if (imp_chn_attr_tmp->crop.enable)
		printf("IMP: Crop enable w:%d h:%d left:%d top:%d\n",
		       imp_chn_attr_tmp->crop.width,
		       imp_chn_attr_tmp->crop.height,
		       imp_chn_attr_tmp->crop.left,
		       imp_chn_attr_tmp->crop.top);

	if (imp_chn_attr_tmp->scaler.enable)
		printf("IMP: Scaler enable w:%d h:%d\n",
		       imp_chn_attr_tmp->scaler.outwidth,
		       imp_chn_attr_tmp->scaler.outheight);

}

static int imp_sdk_yuv_init(int chn_index)
{
	int ret,i = 0;
	int chn_ivdc = 0;

	/*********** Step1 Framesource init Start***********/
	i = chn_index;
	if (chn[i].enable) {
		ret = IMP_FrameSource_CreateChn(chn[i].index, &chn[i].fs_chn_attr);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_FrameSource_CreateChn(chn%d) error !\n", chn[i].index);
			return -1;
		}

		ret = IMP_FrameSource_SetChnAttr(chn[i].index, &chn[i].fs_chn_attr);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_FrameSource_SetChnAttr(chn%d) error !\n", chn[i].index);
			return -1;
		}
	} else {
		printf("framesource channel %d is not enable!\n",chn_index);
		return -1;
	}

	if(ivdc_mode && chn_index == CH0_INDEX) {

		chn_ivdc = CH1_INDEX;
		/*chn0 参数 赋值给ch1 */
		IMPFSChnAttr *fs_chn_attr = &chn[chn_index].fs_chn_attr;
		IMPFSChnAttr *fs_chn_attr_tmp = &chn[chn_ivdc].fs_chn_attr; /* chn1 */

		memcpy((char *)fs_chn_attr_tmp, (char *)fs_chn_attr, sizeof(IMPFSChnAttr));

		i = chn_ivdc;
		if (chn[i].enable) {
			ret = IMP_FrameSource_CreateChn(chn[i].index, &chn[i].fs_chn_attr);
			if (ret < 0)
			{
				IMP_LOG_ERR(TAG, "IMP_FrameSource_CreateChn(chn%d) error !\n", chn[i].index);
				return -1;
			}

			ret = IMP_FrameSource_SetChnAttr(chn[i].index, &chn[i].fs_chn_attr);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_FrameSource_SetChnAttr(chn%d) error !\n", chn[i].index);
				return -1;
			}
		}
	}
	/*********** Step1 Framesource init End***********/


	/*********** Step2 Stream On Start***********/
	i = chn_index;
	ret = IMP_FrameSource_EnableChn(chn[i].index);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_FrameSource_EnableChn(%d) error: %d\n", ret, chn[i].index);
		return -1;
	}

	if(ivdc_mode && chn_index == CH0_INDEX) {
		i = chn_ivdc;
		ret = IMP_FrameSource_EnableChn(chn[i].index);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_FrameSource_EnableChn(%d) error: %d\n", ret, chn[i].index);
			return -1;
		}
		chn_index = chn_ivdc;
	}
	/*********** Step2 Stream On End***********/


	/*********** Step3 Set depth Start***********/
	ret = IMP_FrameSource_SetFrameDepth(chn_index, 1);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "%s(%d):IMP_FrameSource_SetFrameDepth failed\n", __func__, __LINE__);
		return -1;
	}
	/*********** Step3 Set depth End***********/

	return 0;
}

static int imp_sdk_yuv_deinit(int chn_index)
{
	int ret, i = 0;

	/*********** Step1 Stream Off Start***********/
	if (ivdc_mode && chn_index == CH0_INDEX) {
		i = CH1_INDEX;
		if (chn[i].enable) {
			ret = IMP_FrameSource_DisableChn(chn[i].index);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_FrameSource_DisableChn(%d) error: %d\n", ret, chn[i].index);
				return -1;
			}
		}
	}

	i = chn_index;
	if (chn[i].enable) {
		ret = IMP_FrameSource_DisableChn(chn[i].index);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_FrameSource_DisableChn(%d) error: %d\n", ret, chn[i].index);
			return -1;
		}
	}
	/*********** Step1 Stream Off End***********/


	/*********** Step2 Destroy channel Start***********/
	if (ivdc_mode && chn_index == CH0_INDEX) {
		i = CH1_INDEX;
		ret = IMP_FrameSource_DestroyChn(chn[i].index);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_FrameSource_DestroyChn(%d) error: %d\n", chn[i].index, ret);
			return -1;
		}
	}

	i = chn_index;
	ret = IMP_FrameSource_DestroyChn(chn[i].index);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_FrameSource_DestroyChn(%d) error: %d\n", chn[i].index, ret);
		return -1;
	}
	/*********** Step2 Destroy channel End***********/

	return 0;
}

static int imp_sdk_mjpg_init(int chn_index)
{
	int i, ret;

	/*********** Step1 Framesource init Start***********/
	i = chn_index;
	if (chn[i].enable) {
		ret = IMP_FrameSource_CreateChn(chn[i].index, &chn[i].fs_chn_attr);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_FrameSource_CreateChn(chn%d) error !\n", chn[i].index);
			return -1;
		}

		ret = IMP_FrameSource_SetChnAttr(chn[i].index, &chn[i].fs_chn_attr);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_FrameSource_SetChnAttr(chn%d) error !\n", chn[i].index);
			return -1;
		}
	} else {
		printf("framesource channel %d is not enable!\n",chn_index);
		return -1;
	}
	/*********** Step1 Framesource init End***********/


	/*********** Step2 Encoder init Start***********/
	ret = sample_jpeg_encoder_init(chn_index);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Encoder init failed\n");
		printf("Encoder init failed\n");
		return -1;
	}
	/*********** Step2 Encoder init End***********/


	/*********** Step3 Bind Start***********/
#ifdef  IPU_OSD_ENABLE
	ret = sample_ipu_osd_init(0, &chn[i].framesource_chn, &chn[i].imp_encoder);
	if (ret < 0) {
		printf("sample OSD init failed\n");
		return -1;
	}
#else
	ret = IMP_System_Bind(&chn[i].framesource_chn, &chn[i].imp_encoder);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Bind FrameSource channel%d and Encoder failed\n",i);
		return -1;
	}
#endif

	/*********** Step3 Bind End***********/


	/*********** Step4 Stream On Start***********/
	ret = IMP_FrameSource_EnableChn(chn[i].index);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_FrameSource_EnableChn(%d) error: %d\n", ret, chn[i].index);
		return -1;
	}
	/*********** Step4 Stream On End***********/

	ret = IMP_Encoder_StartRecvPic(ENCODER_MJPEG_CHANNEL_INDEX_PLUS + chn[i].index);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_Encoder_StartRecvPic(%d) failed\n", ENCODER_MJPEG_CHANNEL_INDEX_PLUS + chn[i].index);
		return -1;
	}

	return 0;
}

static int imp_sdk_mjpg_deinit(int chn_index)
{
	int i, ret;

	/*********** Step1 Stream Off Start***********/
	i = chn_index;
	if (chn[i].enable) {
		ret = IMP_Encoder_StopRecvPic(ENCODER_MJPEG_CHANNEL_INDEX_PLUS + chn[i].index);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_Encoder_StopRecvPic(%d) failed\n", ENCODER_MJPEG_CHANNEL_INDEX_PLUS + chn[i].index);
			return -1;
		}
	} else {
		printf("framesource channel %d is not enable!\n",chn_index);
		return -1;
	}

	ret = IMP_FrameSource_DisableChn(chn[i].index);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_FrameSource_DisableChn(%d) error: %d\n", ret, chn[i].index);
		return -1;
	}
	/*********** Step1 Stream Off End***********/


	/*********** Step2 UnBind Start***********/
#ifdef IPU_OSD_ENABLE
	ret = sample_ipu_osd_exit(&chn[i].framesource_chn, &chn[i].imp_encoder);
	if (ret < 0) {
		printf("sample OSD exit failed\n");
		return -1;
	}
#else
	ret = IMP_System_UnBind(&chn[i].framesource_chn, &chn[i].imp_encoder);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "UnBind FrameSource channel%d and Encoder failed\n",i);
		return -1;
	}
#endif
	printf("unbind encoder finished\n");
	/*********** Step2 UnBind End***********/


	/*********** Step3 Encoder exit Start***********/
	ret = sample_jpeg_encoder_exit(chn_index);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Encoder exit failed\n");
		return -1;
	}
	/*********** Step3 Encoder exit End***********/


	/*********** Step4 FrameSource exit Start***********/
	ret = IMP_FrameSource_DestroyChn(chn[i].index);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_FrameSource_DestroyChn(%d) error: %d\n", chn[i].index, ret);
		return -1;
	}
	/*********** Step4 FrameSource exit End***********/

	return 0;
}

static int imp_sdk_h264_init(int chn_index)
{
	int i, ret;

	/*********** Step1 Framesource init Start***********/
	i = chn_index;
	if (chn[i].enable) {
		ret = IMP_FrameSource_CreateChn(chn[i].index, &chn[i].fs_chn_attr);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_FrameSource_CreateChn(chn%d) error !\n", chn[i].index);
			return -1;
		}

		ret = IMP_FrameSource_SetChnAttr(chn[i].index, &chn[i].fs_chn_attr);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_FrameSource_SetChnAttr(chn%d) error !\n", chn[i].index);
			return -1;
		}
	} else {
		printf("framesource channel %d is not enable!\n",chn_index);
		return -1;
	}
	/*********** Step1 Framesource init End***********/


	/*********** Step2 Encoder init Start***********/
	ret = sample_h264_encoder_init(chn_index);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Encoder init failed\n");
		printf("Encoder init failed\n");
		return -1;
	}
	/*********** Step2 Encoder init End***********/


	/*********** Step3 Bind Start***********/
#ifdef  IPU_OSD_ENABLE
	ret = sample_ipu_osd_init(0, &chn[i].framesource_chn, &chn[i].imp_encoder);
	if (ret < 0) {
		printf("sample OSD init failed\n");
		return -1;
	}

#else
	ret = IMP_System_Bind(&chn[i].framesource_chn, &chn[i].imp_encoder);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Bind FrameSource channel%d and Encoder failed\n",i);
		return -1;
	}
#endif
	/*********** Step3 Bind End***********/


	/*********** Step4 Stream On Start***********/
	ret = IMP_FrameSource_EnableChn(chn[i].index);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_FrameSource_EnableChn(%d) error: %d\n", ret, chn[i].index);
		return -1;
	}
	/*********** Step4 Stream On End***********/

	ret = IMP_Encoder_StartRecvPic(chn[i].index);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_Encoder_StartRecvPic(%d) failed\n", chn[i].index);
		return -1;
	}

	return 0;
}

static int imp_sdk_h264_deinit(int chn_index)
{
	int i, ret;

	/*********** Step1 Stream Off Start***********/
	i = chn_index;
	if (chn[i].enable) {
		ret = IMP_Encoder_StopRecvPic(chn[i].index);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_Encoder_StopRecvPic(%d) failed\n", chn[i].index);
			return -1;
		}
	} else {
		printf("framesource channel %d is not enable!\n",chn_index);
		return -1;
	}
	printf("---stop h264 finished---\n");

	ret = IMP_FrameSource_DisableChn(chn[i].index);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_FrameSource_DisableChn(%d) error: %d\n", ret, chn[i].index);
		return -1;
	}
	/*********** Step1 Stream Off End***********/


	/*********** Step2 UnBind Start***********/
#ifdef IPU_OSD_ENABLE
	ret = sample_ipu_osd_exit(&chn[i].framesource_chn, &chn[i].imp_encoder);
	if (ret < 0) {
		printf("sample OSD exit failed\n");
		return -1;
	}
#else
	ret = IMP_System_UnBind(&chn[i].framesource_chn, &chn[i].imp_encoder);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "UnBind FrameSource channel%d and Encoder failed\n",i);
		return -1;
	}
#endif
	printf("unbind encoder finished\n");
	/*********** Step2 UnBind End***********/


	/*********** Step3 Encoder exit Start***********/
	ret = sample_h264_encoder_exit(chn_index);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Encoder exit failed\n");
		return -1;
	}
	/*********** Step3 Encoder exit End***********/


	/*********** Step4 FrameSource exit Start***********/
	ret = IMP_FrameSource_DestroyChn(chn[i].index);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_FrameSource_DestroyChn(%d) error: %d\n", chn[i].index, ret);
		return -1;
	}
	/*********** Step4 FrameSource exit End***********/

	return 0;
}

int imp_sdk_init(int video_index, int format, int width, int height)
{
	int ret = 0;

	int chn_index = ((video_index < 1) ? UVC_VIDEO_CH : UVC2_VIDEO_CH);

	printf("IMP SDK init, video_index = %d!\n", video_index);

	/* frame_format for sample_get_xxx_snap */
	frame_format[video_index] = format;

	/* Frame reset */
	imp_isp_scaler(video_index, width, height);

	switch (format) {
	case V4L2_PIX_FMT_YUYV:
	case V4L2_PIX_FMT_NV12:
	case V4L2_PIX_FMT_GREY:
		ret = imp_sdk_yuv_init(chn_index);
		break;
	case V4L2_PIX_FMT_MJPEG:
		ret = imp_sdk_mjpg_init(chn_index);
		break;
	case V4L2_PIX_FMT_H265:
	case V4L2_PIX_FMT_H264:
		ret = imp_sdk_h264_init(chn_index);
		break;
	default:
		break;
	}

	if(ret < 0) {
		printf("IMP SDK[%d] init ...failed!\n",video_index);
		return -1;
	}
	printf("IMP SDK[%d] init ...ok!\n",video_index);

	return 0;
}

int imp_sdk_deinit(int video_index, int format)
{
	int ret = 0;
	int chn_index = ((video_index < 1) ? UVC_VIDEO_CH : UVC2_VIDEO_CH);

	switch(format) {
	case V4L2_PIX_FMT_YUYV:
	case V4L2_PIX_FMT_NV12:
	case V4L2_PIX_FMT_GREY:
		ret = imp_sdk_yuv_deinit(chn_index);
		break;
	case V4L2_PIX_FMT_MJPEG:
		ret = imp_sdk_mjpg_deinit(chn_index);
		break;
	case V4L2_PIX_FMT_H265:
	case V4L2_PIX_FMT_H264:
		ret = imp_sdk_h264_deinit(chn_index);
		break;
	default:
		break;
	}
	if(ret < 0) {
		printf("imp sdk deinit failed\n");
		return -1;
	}

	return 0;
}

