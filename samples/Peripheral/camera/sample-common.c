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

#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_framesource.h>
#include <imp/imp_encoder.h>
#include <imp/imp_isp.h>
#include <imp/imp_osd.h>
//#include "logodata_100x100_bgra.h"
#include "sample-common.h"
#include "imp/imp-common.h"


#include "cm_config.h"
#define TAG "Sample-Common"

#define DEFAULE_IMAGE_ROTATE 1  /*0:不旋转, 1:旋转90度*/


/*全局配置*/
static ImageAttributesConfigs g_configs = {0};


static const int S_RC_METHOD = ENC_RC_MODE_CBR;
int direct_switch = 0;
int gosd_enable = 0; /* 1: ipu osd, 2: isp osd, 3: ipu osd and isp osd */
//#define SHOW_FRM_BITRATE
#ifdef SHOW_FRM_BITRATE
#define FRM_BIT_RATE_TIME 2
#define STREAM_TYPE_NUM 3
static int frmrate_sp[STREAM_TYPE_NUM] = { 0 };
static int statime_sp[STREAM_TYPE_NUM] = { 0 };
static int bitrate_sp[STREAM_TYPE_NUM] = { 0 };
#endif

struct chn_conf chn[FS_CHN_NUM] = {
	{
		.index = CH0_INDEX,
		.enable = CHN0_EN,
        .payloadType = PT_JPEG,
		.fs_chn_attr = {
			.pixFmt = PIX_FMT_NV12,	/*像素类型*/
			.outFrmRateNum = SENSOR_FRAME_RATE_NUM,	/*输出帧率分子*/
			.outFrmRateDen = SENSOR_FRAME_RATE_DEN,	/*输出帧率分母*/
			.nrVBs = 2,	/*帧缓冲区数量*/
			.type = FS_PHY_CHANNEL,	/*物理通道类型*/

			/*裁剪*/
			.crop.enable = 1,
			.crop.top = 0,
			.crop.left = 0,
			.crop.width = SENSOR_WIDTH,
			.crop.height = SENSOR_HEIGHT,

			/*缩放*/
			.scaler.enable = 0,
			.scaler.outwidth = SENSOR_WIDTH,
			.scaler.outheight = SENSOR_HEIGHT,

			.picWidth = SENSOR_WIDTH,
			.picHeight = SENSOR_HEIGHT,
		   },
		.framesource_chn =	{ DEV_ID_FS, CH0_INDEX, 0},
		.imp_encoder = { DEV_ID_ENC, CH0_INDEX, 0},
	},
	{
		.index = CH1_INDEX,
		.enable = CHN1_EN,
        .payloadType = PT_H264,
		.fs_chn_attr = {
			.pixFmt = PIX_FMT_NV12,
			.outFrmRateNum = SENSOR_FRAME_RATE_NUM,
			.outFrmRateDen = SENSOR_FRAME_RATE_DEN,
			.nrVBs = 2,
			.type = FS_PHY_CHANNEL,

			.crop.enable = 1,
			.crop.top = 0,
			.crop.left = 0,
			.crop.width = SENSOR_WIDTH,
			.crop.height = SENSOR_HEIGHT,

			.scaler.enable = 1,
			.scaler.outwidth = SENSOR_WIDTH_SECOND,
			.scaler.outheight = SENSOR_HEIGHT_SECOND,

			.picWidth = SENSOR_WIDTH_SECOND,
			.picHeight = SENSOR_HEIGHT_SECOND,
		   },
		.framesource_chn =	{ DEV_ID_FS, CH1_INDEX, 0},
		.imp_encoder = { DEV_ID_ENC, CH1_INDEX, 0},
	},
	{
		.index = CH2_INDEX,
		.enable = CHN2_EN,
        .payloadType = PT_H264,
		.fs_chn_attr = {
			.pixFmt = PIX_FMT_NV12,
			.outFrmRateNum = SENSOR_FRAME_RATE_NUM,
			.outFrmRateDen = SENSOR_FRAME_RATE_DEN,
			.nrVBs = 2,
			.type = FS_PHY_CHANNEL,

			.crop.enable = 1,
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
		.framesource_chn =	{ DEV_ID_FS, CH2_INDEX, 0},
		.imp_encoder = { DEV_ID_ENC, CH2_INDEX, 0},
	},
	{
		.index = CH3_INDEX,
		.enable = CHN3_EN,
        .payloadType = PT_H264,
		.fs_chn_attr = {
			.pixFmt = PIX_FMT_NV12,
			.outFrmRateNum = SENSOR_FRAME_RATE_NUM,
			.outFrmRateDen = SENSOR_FRAME_RATE_DEN,
			.nrVBs = 2,
			.type = FS_PHY_CHANNEL,

			.crop.enable = 0,
			.crop.top = 0,
			.crop.left = 0,
			.crop.width = SENSOR_WIDTH,
			.crop.height = SENSOR_HEIGHT,

			.scaler.enable = 1,
			.scaler.outwidth = SENSOR_WIDTH,
			.scaler.outheight = SENSOR_HEIGHT,

			.picWidth = SENSOR_WIDTH,
			.picHeight = SENSOR_HEIGHT,
		   },
		.framesource_chn =	{ DEV_ID_FS, CH3_INDEX, 0},
		.imp_encoder = { DEV_ID_ENC, CH3_INDEX, 0},
	},
};

struct chn_conf chn_ext_hsv[1] = {
	{
		.fs_chn_attr = {
			.pixFmt = PIX_FMT_HSV,
			.outFrmRateNum = SENSOR_FRAME_RATE_NUM,
			.outFrmRateDen = SENSOR_FRAME_RATE_DEN,
			.nrVBs = 2,
			.type = FS_EXT_CHANNEL,

			.crop.enable = 0,
			.crop.top = 0,
			.crop.left = 0,
			.crop.width = SENSOR_WIDTH,
			.crop.height = SENSOR_HEIGHT,

			.scaler.enable = 1,
			.scaler.outwidth = SENSOR_WIDTH_SECOND,
			.scaler.outheight = SENSOR_HEIGHT_SECOND,

			.picWidth = SENSOR_WIDTH_SECOND,
			.picHeight = SENSOR_HEIGHT_SECOND,
		},
	},
};

struct chn_conf chn_ext_rgba[1] = {
	{
		.fs_chn_attr = {
			.pixFmt = PIX_FMT_RGBA,
			.outFrmRateNum = SENSOR_FRAME_RATE_NUM,
			.outFrmRateDen = SENSOR_FRAME_RATE_DEN,
			.nrVBs = 2,
			.type = FS_EXT_CHANNEL,

			.crop.enable = 0,
			.crop.top = 0,
			.crop.left = 0,
			.crop.width = SENSOR_WIDTH,
			.crop.height = SENSOR_HEIGHT,

			.scaler.enable = 1,
			.scaler.outwidth = SENSOR_WIDTH_SECOND,
			.scaler.outheight = SENSOR_HEIGHT_SECOND,

			.picWidth = SENSOR_WIDTH_SECOND,
			.picHeight = SENSOR_HEIGHT_SECOND,
		},
	},
};



IMPSensorInfo sensor_info;
IMPSensorInfo sensor_infos1;

int sample_system_init(ImageAttributesConfigs configs)
{
	int ret = 0;
	g_configs = configs;

	// /* isp osd and ipu osd buffer size set */
	// if(1 == gosd_enable) { /* only use ipu osd */
	// 	IMP_OSD_SetPoolSize(512*1024);
	// } else if(2 == gosd_enable) { /* only use isp osd */
	// 	IMP_ISP_Tuning_SetOsdPoolSize(512 * 1024);
	// }else if(3 == gosd_enable) { /* use ipu osd and isp osd */
	// 	IMP_OSD_SetPoolSize(512*1024);
	// 	IMP_ISP_Tuning_SetOsdPoolSize(512 * 1024);
	// } else {
	// 	IMP_OSD_SetPoolSize(512*1024);
	// 	IMP_ISP_Tuning_SetOsdPoolSize(512 * 1024);
    // }

	memset(&sensor_info, 0, sizeof(IMPSensorInfo));
	memcpy(sensor_info.name, SENSOR_NAME, sizeof(SENSOR_NAME));
	sensor_info.cbus_type = SENSOR_CUBS_TYPE;

	memcpy(sensor_info.i2c.type, SENSOR_NAME, sizeof(SENSOR_NAME));
	sensor_info.i2c.addr = SENSOR_I2C_ADDR;
	sensor_info.i2c.i2c_adapter_id = SENSOR_I2C_ADAPTER_ID;

#ifdef SECSENSOR
	memset(&sensor_infos1, 0, sizeof(IMPSensorInfo));
	memcpy(sensor_infos1.name, SENSOR_NAMES1, sizeof(SENSOR_NAMES1));
	sensor_infos1.cbus_type = SENSOR_CUBS_TYPE;
	sensor_infos1.sensor_id = 1;

	memcpy(sensor_infos1.i2c.type, SENSOR_NAMES1, sizeof(SENSOR_NAMES1));
	sensor_infos1.i2c.addr = SENSOR_I2C_ADDRS1;
	sensor_infos1.i2c.i2c_adapter_id = SENSOR_I2C_ADAPTER_ID;
#endif

	IMP_LOG_DBG(TAG, "sample_system_init start\n");

	ret = IMP_ISP_Open();
	if(ret < 0){
		IMP_LOG_ERR(TAG, "failed to open ISP\n");
		return -1;
	}

	ret = IMP_ISP_AddSensor(&sensor_info);
	if(ret < 0){
		IMP_LOG_ERR(TAG, "failed to AddSensor\n");
		return -1;
	}

#ifdef SECSENSOR
	ret = IMP_ISP_AddSensor(&sensor_infos1);
	if(ret < 0){
		IMP_LOG_ERR(TAG, "failed to AddSensor\n");
		return -1;
	}
#endif

	ret = IMP_ISP_EnableSensor();
	if(ret < 0){
		IMP_LOG_ERR(TAG, "failed to EnableSensor\n");
		return -1;
	}

	ret = IMP_System_Init();
	if(ret < 0){
		IMP_LOG_ERR(TAG, "IMP_System_Init failed\n");
		return -1;
	}

	/* enable turning, to debug graphics */
	ret = IMP_ISP_EnableTuning();
	if(ret < 0){
		IMP_LOG_ERR(TAG, "IMP_ISP_EnableTuning failed\n");
		return -1;
	}
	
    IMP_ISP_Tuning_SetContrast(128);
    IMP_ISP_Tuning_SetSharpness(128);
    IMP_ISP_Tuning_SetSaturation(128);
    IMP_ISP_Tuning_SetBrightness(configs.luminance);

	/*设定宽高属性*/
	uint16_t width = g_configs.width;
	uint16_t height = g_configs.height;
	/*设置图片旋转*/
#if DEFAULE_IMAGE_ROTATE
	ret = IMP_FrameSource_SetChnRotate(0, 1, width, height);
	if (ret) {
		IMP_LOG_ERR(TAG, "IMP_FrameSource_SetChnRotate error !\n");
	} else {
		IMP_LOG_INFO(TAG, "IMP_FrameSource_SetChnRotate(%d-%d-%d-%d) is success!\n", 0, 1, height, width);
	}
#endif


	/*只使用通道1 , 设置其宽高属性*/
	chn[0].fs_chn_attr.picWidth = width;
	chn[0].fs_chn_attr.picHeight = height;

	chn[0].fs_chn_attr.crop.width = width;
	chn[0].fs_chn_attr.crop.height = height;

	chn[0].fs_chn_attr.scaler.outwidth = width;
	chn[0].fs_chn_attr.scaler.outheight = height;


#if 1
    ret = IMP_ISP_Tuning_SetISPRunningMode(IMPISP_RUNNING_MODE_DAY);
    if (ret < 0){
        IMP_LOG_ERR(TAG, "failed to set running mode\n");
        return -1;
    }
#endif
#if 0
    ret = IMP_ISP_Tuning_SetSensorFPS(SENSOR_FRAME_RATE_NUM, SENSOR_FRAME_RATE_DEN);
    if (ret < 0){
        IMP_LOG_ERR(TAG, "failed to set sensor fps\n");
        return -1;
    }
#endif
	IMP_LOG_DBG(TAG, "ImpSystemInit success\n");

	return 0;
}

int sample_system_exit()
{
	int ret = 0;

	IMP_LOG_DBG(TAG, "sample_system_exit start\n");


	IMP_System_Exit();

	ret = IMP_ISP_DisableSensor();
	if(ret < 0){
		IMP_LOG_ERR(TAG, "failed to EnableSensor\n");
		return -1;
	}

#ifdef SECSENSOR
	ret = IMP_ISP_DelSensor(&sensor_infos1);
	if(ret < 0){
		IMP_LOG_ERR(TAG, "failed to AddSensor\n");
		return -1;
	}
#endif

	ret = IMP_ISP_DelSensor(&sensor_info);
	if(ret < 0){
		IMP_LOG_ERR(TAG, "failed to AddSensor\n");
		return -1;
	}

	ret = IMP_ISP_DisableTuning();
	if(ret < 0){
		IMP_LOG_ERR(TAG, "IMP_ISP_DisableTuning failed\n");
		return -1;
	}

	if(IMP_ISP_Close()){
		IMP_LOG_ERR(TAG, "failed to open ISP\n");
		return -1;
	}

	IMP_LOG_DBG(TAG, " wind_sample_system_exit success\n");

	return 0;
}

int sample_framesource_streamon()
{
	int ret = 0, i = 0;
	/* Enable channels */
	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			ret = IMP_FrameSource_EnableChn(chn[i].index);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_FrameSource_EnableChn(%d) error: %d\n", ret, chn[i].index);
				return -1;
			}
		}
	}
	return 0;
}




int sample_framesource_streamoff()
{
	int ret = 0, i = 0;
	/* Enable channels */
	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable){
			ret = IMP_FrameSource_DisableChn(chn[i].index);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_FrameSource_DisableChn(%d) error: %d\n", ret, chn[i].index);
				return -1;
			}
		}
	}
	return 0;
}




int sample_framesource_init()
{
	int i, ret;
	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			ret = IMP_FrameSource_CreateChn(chn[i].index, &chn[i].fs_chn_attr);
			if(ret < 0){
				IMP_LOG_ERR(TAG, "IMP_FrameSource_CreateChn(chn%d) error !\n", chn[i].index);
				return -1;
			}

			ret = IMP_FrameSource_SetChnAttr(chn[i].index, &chn[i].fs_chn_attr);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_FrameSource_SetChnAttr(chn%d) error !\n",  chn[i].index);
				return -1;
			}
		}
	}

	return 0;
}

int sample_framesource_exit()
{
	int ret,i;

	for (i = 0; i <  FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			/*Destroy channel */
			ret = IMP_FrameSource_DestroyChn(chn[i].index);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_FrameSource_DestroyChn(%d) error: %d\n", chn[i].index, ret);
				return -1;
			}
		}
	}
	return 0;
}

static const int jpeg_chroma_quantizer[64] = {
	17, 18, 24, 47, 99, 99, 99, 99,
	18, 21, 26, 66, 99, 99, 99, 99,
	24, 26, 56, 99, 99, 99, 99, 99,
	47, 66, 99, 99, 99, 99, 99, 99,
	99, 99, 99, 99, 99, 99, 99, 99,
	99, 99, 99, 99, 99, 99, 99, 99,
	99, 99, 99, 99, 99, 99, 99, 99,
	99, 99, 99, 99, 99, 99, 99, 99
};

static const int jpeg_luma_quantizer[64] = {
	16, 11, 10, 16, 24, 40, 51, 61,
	12, 12, 14, 19, 26, 58, 60, 55,
	14, 13, 16, 24, 40, 57, 69, 56,
	14, 17, 22, 29, 51, 87, 80, 62,
	18, 22, 37, 56, 68, 109, 103, 77,
	24, 35, 55, 64, 81, 104, 113, 92,
	49, 64, 78, 87, 103, 121, 120, 101,
	72, 92, 95, 98, 112, 100, 103, 99
};



/* 生成jpeg量化表 */
static void MakeTables(int q, uint8_t *lqt, uint8_t *cqt)
{
	int i;
	int factor = q;
	if (q < 1) factor = 1;
	if (q > 99) factor = 99;
	if (q < 50)
		q = 5000 / factor;
	else
		q = 200 - factor*2;
	for (i=0; i < 64; i++) {
		int lq = (jpeg_luma_quantizer[i] * q + 50) / 100;
		int cq = (jpeg_chroma_quantizer[i] * q + 50) / 100;
		/* Limit the quantizers to 1 <= q <= 255 */
		if (lq < 1) lq = 1;
		else if (lq > 255) lq = 255;
		lqt[i] = lq;
		if (cq < 1) cq = 1;
		else if (cq > 255) cq = 255;
		cqt[i] = cq;
	}
}

/* 初始化jpeg编码器 */
int sample_jpeg_init()
{
	int i, ret;
	IMPEncoderAttr *enc_attr;
	IMPEncoderCHNAttr channel_attr;
	IMPFSChnAttr *imp_chn_attr_tmp;

	for (i = 0; i <  FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			imp_chn_attr_tmp = &chn[i].fs_chn_attr;
			memset(&channel_attr, 0, sizeof(IMPEncoderCHNAttr));
			enc_attr = &channel_attr.encAttr;
			enc_attr->enType = PT_JPEG;
			enc_attr->bufSize = 0;
			enc_attr->profile = 0;
			enc_attr->picWidth = imp_chn_attr_tmp->picWidth;
			enc_attr->picHeight = imp_chn_attr_tmp->picHeight;
			
/*如果图片旋转，则交换宽高属性*/
#if DEFAULE_IMAGE_ROTATE
			enc_attr->picWidth = imp_chn_attr_tmp->picHeight;
			enc_attr->picHeight = imp_chn_attr_tmp->picWidth;
#endif

			/* Create Channel */
			if(direct_switch == 1) {
				if (0 == chn[i].index) channel_attr.bEnableIvdc = true;
			}

			ret = IMP_Encoder_CreateChn(3 + chn[i].index, &channel_attr);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_Encoder_CreateChn(%d) error: %d\n",
							chn[i].index, ret);
				return -1;
			}

			/* Resigter Channel */
			ret = IMP_Encoder_RegisterChn(i, 3 + chn[i].index);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_Encoder_RegisterChn(0, %d) error: %d\n",
							chn[i].index, ret);
				return -1;
			}
		}
	}

	/*设置jpeg压缩率*/
	IMPEncoderJpegeQl pstJpegeQl;
	uint8_t compression = g_configs.compression;
	/*to do : 判断压缩率合理性？*/
	for (i = 0; i < FS_CHN_NUM; i++) 
	{
		if (chn[i].enable) 
		{
			IMP_Encoder_GetJpegeQl(3+chn[i].index, &pstJpegeQl);
			MakeTables(compression, &(pstJpegeQl.qmem_table[0]), &(pstJpegeQl.qmem_table[64]));
			pstJpegeQl.user_ql_en = 1;
			IMP_Encoder_SetJpegeQl(3+chn[i].index, &pstJpegeQl);
		}
	}
	return 0;
}

int sample_encoder_exit(void)
{
    int ret = 0, i = 0, chnNum = 0;
    IMPEncoderCHNStat chn_stat;

	for (i = 0; i <  FS_CHN_NUM; i++) {
		if (chn[i].enable) {
            if (chn[i].payloadType == PT_JPEG) {
                chnNum = 3 + chn[i].index;
            } else {
                chnNum = chn[i].index;
            }
            memset(&chn_stat, 0, sizeof(IMPEncoderCHNStat));
            ret = IMP_Encoder_Query(chnNum, &chn_stat);
            if (ret < 0) {
                IMP_LOG_ERR(TAG, "IMP_Encoder_Query(%d) error: %d\n", chnNum, ret);
                return -1;
            }

            if (chn_stat.registered) {
                ret = IMP_Encoder_UnRegisterChn(chnNum);
                if (ret < 0) {
                    IMP_LOG_ERR(TAG, "IMP_Encoder_UnRegisterChn(%d) error: %d\n", chnNum, ret);
                    return -1;
                }

                ret = IMP_Encoder_DestroyChn(chnNum);
                if (ret < 0) {
                    IMP_LOG_ERR(TAG, "IMP_Encoder_DestroyChn(%d) error: %d\n", chnNum, ret);
                    return -1;
                }
            }
        }
    }

    return 0;
}


int sample_res_deinit()
{
	int ret, i;
	/* Step.a Stream Off */
	ret = sample_framesource_streamoff();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "FrameSource StreamOff failed\n");
		return -1;
	}

	/* Step.b UnBind */
	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			ret = IMP_System_UnBind(&chn[i].framesource_chn, &chn[i].imp_encoder);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "UnBind FrameSource channel%d and Encoder failed\n",i);
				return -1;
			}
		}
	}

	/* Step.c Encoder exit */
	ret = sample_encoder_exit();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Encoder exit failed\n");
		return -1;
	}

	/* Step.d FrameSource exit */
	ret = sample_framesource_exit();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "FrameSource exit failed\n");
		return -1;
	}
	IMP_LOG_ERR(TAG, "sample_res_deinit scuess\n");

	return 0;
}


int sample_res_init()
{
	int ret, i;

	/* Step.2 FrameSource init */
	ret = sample_framesource_init();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "FrameSource init failed\n");
		return -1;
	}

	/* Step.3 Encoder init */
	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			ret = IMP_Encoder_CreateGroup(chn[i].index);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_Encoder_CreateGroup(%d) error !\n", chn[i].index);
				return -1;
			}
		}
	}

	ret = sample_jpeg_init();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Encoder init failed\n");
		return -1;
	}

	/* Step.4 Bind */
	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			ret = IMP_System_Bind(&chn[i].framesource_chn, &chn[i].imp_encoder);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "Bind FrameSource channel%d and Encoder failed\n",i);
				return -1;
			}
		}
	}

	/* Step.5 Stream On */
	ret = sample_framesource_streamon();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "ImpStreamOn failed\n");
		return -1;
	}
	IMP_LOG_INFO(TAG, "sample_res_init scuess\n");
	return 0;
}

/*设置亮度属性接口*/
int sample_set_attributes_luminance(uint8_t luminance)
{
	IMP_LOG_ERR(TAG, "sample_set_attributes_luminance luminance:%d\n", luminance);
	int ret = 0;
	ret = IMP_ISP_Tuning_SetBrightness(luminance);
	if(ret < 0)
	{
		IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_SetBrightness(%d) error: %d\n", luminance, ret);
		return -1;
	}
	return 0;
}

/*设置压缩率属性接口*/
/*注意 : 是否需要重新初始化fs enc?*/
int sample_set_attributes_compression(uint8_t compression)
{
	IMP_LOG_ERR(TAG, "sample_set_attributes_compression compression:%d\n", compression);
	int ret = 0;
	/*设置jpeg压缩率*/
	IMPEncoderJpegeQl pstJpegeQl;
	uint8_t Compression = compression;
	/*to do : 判断压缩率合理性？*/
	for (int i = 0; i < FS_CHN_NUM; i++) 
	{
		if (chn[i].enable) 
		{
			IMP_Encoder_GetJpegeQl(3+chn[i].index, &pstJpegeQl);
			MakeTables(compression, &(pstJpegeQl.qmem_table[0]), &(pstJpegeQl.qmem_table[64]));
			pstJpegeQl.user_ql_en = 1;
			IMP_Encoder_SetJpegeQl(3+chn[i].index, &pstJpegeQl);
		}
	}
	return 0;
}


/*过滤不合理分辨率

64对齐：1920 1088
			*/
static int is_valid_resolution(uint16_t width, uint16_t height) {
    // 检查宽高比
    // if (width * 3 != height * 16 && width * 4 != height * 3) {
    //     return false; // 不是16:9或4:3
    // }


	 // 检查对齐要求
#if DEFAULE_IMAGE_ROTATE
	/*图片翻转必须是64位对齐的*/
	if (width % 64 != 0 || height % 64 != 0) {
        return -1; // 不满足对齐要求
    }
#else

    if (width % 16 != 0 || height % 16 != 0) {
        return -1; // 不满足对齐要求
    }
#endif


    // 检查最大和最小限制
    if (width < 320 || height < 240 || width > 1920 || height > 1088) {
        return -1; // 超出范围
    }

    return 0; // 合理的分辨率
}



/*设置图像宽高属性接口*/
/*注意 : 需要重新初始化fs enc*/
int sample_set_attributes_resolution(uint16_t width, uint16_t height)
{
	IMP_LOG_ERR(TAG, "sample_set_attributes_resolution width:%d, height:%d\n", width, height);
	int ret = 0;
	ret = is_valid_resolution(width,height);
	if(ret < 0)
	{
		IMP_LOG_ERR(TAG, "resolution is not valid\n");
		return -1;
	}

	ret = sample_res_deinit();
	if(ret < 0)
	{
		IMP_LOG_ERR(TAG, "sample_res_deinit failed\n");
		return -1;
	}
	IMP_LOG_ERR(TAG, "sample res deinit success\n");
	chn[0].fs_chn_attr.picWidth = width;
	chn[0].fs_chn_attr.picHeight = height;

	chn[0].fs_chn_attr.crop.width = width;
	chn[0].fs_chn_attr.crop.height = height;

	chn[0].fs_chn_attr.scaler.outwidth = width;
	chn[0].fs_chn_attr.scaler.outheight = height;

/*如果分辨率设定的值不合理，会创建FS失败*/
	ret = sample_res_init();
	if(ret < 0)
	{
		IMP_LOG_ERR(TAG, "sample_res_init failed\n");
		return -1;
	}
	return 0;
}






