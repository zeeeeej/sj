/*
 * sample-common.h
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 */

#ifndef __SAMPLE_COMMON_H__
#define __SAMPLE_COMMON_H__

#include <imp/imp_common.h>
#include <imp/imp_osd.h>
#include <imp/imp_framesource.h>
#include <imp/imp_isp.h>
#include <unistd.h>
#include "cm_config/cm_config.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

#define SENSOR_FRAME_RATE_NUM		25
#define SENSOR_FRAME_RATE_DEN		1

#define SENSOR_GC2053
//#define SECSENSOR

#if defined SENSOR_AR0141
#define SENSOR_NAME				"ar0141"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR			0x10
#define SENSOR_I2C_ADAPTER_ID   0
#define SENSOR_WIDTH			1280
#define SENSOR_HEIGHT			720
#define CHN0_EN                 1
#define CHN1_EN                 0
#define CHN2_EN                 1
#define CHN3_EN                 0
#define CROP_EN					1
#elif defined SENSOR_OV7725
#define SENSOR_NAME				"ov7725"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR			0x21
#define SENSOR_I2C_ADAPTER_ID   0
#define SENSOR_WIDTH			640
#define SENSOR_HEIGHT			480
#define CHN0_EN                 1
#define CHN1_EN                 0
#define CHN2_EN                 1
#define CHN3_EN                 0
#define CROP_EN					0
#elif defined SENSOR_OV9732
#define SENSOR_NAME				"ov9732"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR			0x36
#define SENSOR_I2C_ADAPTER_ID   0
#define SENSOR_WIDTH			1280
#define SENSOR_HEIGHT			720
#define CHN0_EN                 1
#define CHN1_EN                 0
#define CHN2_EN                 1
#define CHN3_EN                 0
#define CROP_EN					1
#elif defined SENSOR_OV9750
#define SENSOR_NAME				"ov9750"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR			0x36
#define SENSOR_I2C_ADAPTER_ID   0
#define SENSOR_WIDTH			1280
#define SENSOR_HEIGHT			720
#define CHN0_EN                 1
#define CHN1_EN                 0
#define CHN2_EN                 1
#define CHN3_EN                 0
#define CROP_EN					1
#elif defined SENSOR_OV9712
#define SENSOR_NAME				"ov9712"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR			0x30
#define SENSOR_I2C_ADAPTER_ID   0
#define SENSOR_WIDTH			1280
#define SENSOR_HEIGHT			720
#define CHN0_EN                 1
#define CHN1_EN                 0
#define CHN2_EN                 1
#define CHN3_EN                 0
#define CROP_EN					1
#elif defined SENSOR_GC1004
#define SENSOR_NAME				"gc1004"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR			0x3c
#define SENSOR_I2C_ADAPTER_ID   0
#define SENSOR_WIDTH			1280
#define SENSOR_HEIGHT			720
#define CHN0_EN                 1
#define CHN1_EN                 0
#define CHN2_EN                 1
#define CHN3_EN                 0
#define CROP_EN					1
#elif defined SENSOR_JXH42
#define SENSOR_NAME				"jxh42"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR			0x30
#define SENSOR_I2C_ADAPTER_ID   0
#define SENSOR_WIDTH			1280
#define SENSOR_HEIGHT			720
#define CHN0_EN                 1
#define CHN1_EN                 0
#define CHN2_EN                 1
#define CHN3_EN                 0
#define CROP_EN					1
#elif defined SENSOR_SC1035
#define SENSOR_NAME				"sc1035"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR			0x30
#define SENSOR_I2C_ADAPTER_ID   0
#define SENSOR_WIDTH			1280
#define SENSOR_HEIGHT			960
#define CHN0_EN                 1
#define CHN1_EN                 0
#define CHN2_EN                 1
#define CHN3_EN                 0
#define CROP_EN					1
#elif defined SENSOR_OV2710
#define SENSOR_NAME				"ov2710"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR			0x36
#define SENSOR_I2C_ADAPTER_ID   0
#define SENSOR_WIDTH			1920
#define SENSOR_HEIGHT			1080
#define CHN0_EN                 1
#define CHN1_EN                 0
#define CHN2_EN                 1
#define CHN3_EN                 0
#define CROP_EN					1
#elif defined SENSOR_OV2735
#define SENSOR_NAME				"ov2735"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR			0x3c
#define SENSOR_I2C_ADAPTER_ID   0
#define SENSOR_WIDTH			1920
#define SENSOR_HEIGHT			1080
#define CHN0_EN                 1
#define CHN1_EN                 0
#define CHN2_EN                 1
#define CHN3_EN                 0
#define CROP_EN					1
#elif defined SENSOR_OV2735B
#define SENSOR_NAME				"ov2735b"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR			0x3c
#define SENSOR_I2C_ADAPTER_ID   0
#define SENSOR_WIDTH			1920
#define SENSOR_HEIGHT			1080
#define CHN0_EN                 1
#define CHN1_EN                 0
#define CHN2_EN                 1
#define CHN3_EN                 0
#define CROP_EN					1
#elif defined SENSOR_SC2135
#define SENSOR_NAME				"sc2135"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR			0x30
#define SENSOR_I2C_ADAPTER_ID   0
#define SENSOR_WIDTH			1920
#define SENSOR_HEIGHT			1080
#define CHN0_EN                 1
#define CHN1_EN                 0
#define CHN2_EN                 1
#define CHN3_EN                 0
#define CROP_EN					1
#elif defined SENSOR_JXF22
#define SENSOR_NAME				"jxf22"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR			0x40
#define SENSOR_I2C_ADAPTER_ID   0
#define SENSOR_WIDTH			1920
#define SENSOR_HEIGHT			1080
#define CHN0_EN                 1
#define CHN1_EN                 0
#define CHN2_EN                 1
#define CHN3_EN                 0
#define CROP_EN					1
#elif defined SENSOR_JXF23
#define SENSOR_NAME				"jxf23"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR			0x40
#define SENSOR_I2C_ADAPTER_ID   0
#define SENSOR_WIDTH			1920
#define SENSOR_HEIGHT			1080
#define CHN0_EN                 0
#define CHN1_EN                 1
#define CHN2_EN                 0
#define CHN3_EN                 0
#define CROP_EN                 0
#elif defined SENSOR_JXF28
#define SENSOR_NAME				"jxf28"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR			0x40
#define SENSOR_I2C_ADAPTER_ID   0
#define SENSOR_WIDTH			1920
#define SENSOR_HEIGHT			1080
#define CHN0_EN                 1
#define CHN1_EN                 0
#define CHN2_EN                 1
#define CHN3_EN                 0
#define CROP_EN					1
#elif defined SENSOR_SC2310
#define SENSOR_NAME             "sc2310"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR         0x30
#define SENSOR_I2C_ADAPTER_ID   0
#define SENSOR_WIDTH            1920
#define SENSOR_HEIGHT           1080
#define CHN0_EN                 1
#define CHN1_EN                 0
#define CHN2_EN                 0
#define CHN3_EN                 0
#define CROP_EN                 0
#elif defined SENSOR_GC2063
#define SENSOR_NAME             "gc2063"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR         0x37
#define SENSOR_I2C_ADAPTER_ID   1
#define SENSOR_WIDTH            1920
#define SENSOR_HEIGHT           1080
#define CHN0_EN                 1
#define CHN1_EN                 0
#define CHN2_EN                 0
#define CHN3_EN                 0
#define CROP_EN                 0
#elif defined SENSOR_GC2053
#define SENSOR_NAME             "gc2053"
#define SENSOR_NAMES1           "gc2053s1"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR         0x37
#define SENSOR_I2C_ADDRS1       0x3f
#define SENSOR_I2C_ADAPTER_ID   0
#define SENSOR_WIDTH            1920
#define SENSOR_HEIGHT           1080
#define CHN0_EN                 1
#define CHN1_EN                 0
#define CHN2_EN					0
#define CHN3_EN                 0
#define CROP_EN                 0
#elif defined SENSOR_IMX327
#define SENSOR_NAME             "imx327"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR         0x1a
#define SENSOR_I2C_ADAPTER_ID   0
#define SENSOR_WIDTH            1920
#define SENSOR_HEIGHT           1080
#define CHN0_EN                 1
#define CHN1_EN                 0
#define CHN2_EN                 0
#define CHN3_EN                 0
#define CROP_EN                 0
#elif defined SENSOR_JXF37
#define SENSOR_NAME             "jxf37"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR         0x40
#define SENSOR_I2C_ADAPTER_ID   0
#define SENSOR_WIDTH            1920
#define SENSOR_HEIGHT           1080
#define CHN0_EN                 1
#define CHN1_EN                 0
#define CHN2_EN                 0
#define CHN3_EN                 0
#define CROP_EN                 0
#elif defined SENSOR_SC200AI
#define SENSOR_NAME             "sc200ai"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR         0x30
#define SENSOR_I2C_ADAPTER_ID   0
#define SENSOR_WIDTH            1920
#define SENSOR_HEIGHT           1080
#define CHN0_EN                 1
#define CHN1_EN                 0
#define CHN2_EN                 0
#define CHN3_EN                 0
#define CROP_EN                 0
#elif defined SENSOR_SC301IoT
#define SENSOR_NAME             "sc301IoT"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR         0x30
#define SENSOR_I2C_ADAPTER_ID   0
#define SENSOR_WIDTH            2048
#define SENSOR_HEIGHT           1536
#define CHN0_EN                 1
#define CHN1_EN                 0
#define CHN2_EN                 0
#define CHN3_EN                 0
#define CROP_EN                 0
#endif

#define SENSOR_WIDTH_SECOND		640
#define SENSOR_HEIGHT_SECOND	360

#define SENSOR_WIDTH_THIRD		1280
#define SENSOR_HEIGHT_THIRD	    720

#define BITRATE_720P_Kbs        1000

#define NR_FRAMES_TO_SAVE		200
#define NR_JPEG_TO_SAVE			20
#define STREAM_BUFFER_SIZE		(1 * 1024 * 1024)

#define ENC_VIDEO_CHANNEL		0
#define ENC_JPEG_CHANNEL		1

#define STREAM_FILE_PATH_PREFIX		"/tmp"
#define SNAP_FILE_PATH_PREFIX		"/tmp"

#define OSD_REGION_WIDTH		16
#define OSD_REGION_HEIGHT		34
#define OSD_REGION_WIDTH_SEC		8
#define OSD_REGION_HEIGHT_SEC   	18


#define SLEEP_TIME			1

#define FS_CHN_NUM			4  //MIN 1,MAX 4
#define IVS_CHN_ID          2

#define CH0_INDEX  0
#define CH1_INDEX  1
#define CH2_INDEX  2
#define CH3_INDEX  3	//ext chn

#define CHN_ENABLE 1
#define CHN_DISABLE 0

/*#define SUPPORT_RGB555LE*/

struct chn_conf{
	unsigned int index;//0 for main channel ,1 for second channel
	unsigned int enable;
    IMPPayloadType  payloadType;
	IMPFSChnAttr fs_chn_attr;
	IMPCell framesource_chn;
	IMPCell imp_encoder;
};


/*属性配置值*/
typedef struct {
    uint8_t luminance;      // 亮度属性
    uint8_t compression;    // 压缩率属性
    uint8_t contrast;       // 对比度属性
    uint16_t height;        // 图像高度
    uint16_t width;         // 图像宽度
} ImageAttributesConfigs;


#define  CHN_NUM  ARRAY_SIZE(chn)
#define SENSOR_WIDTH 1920
#define SENSOR_HEIGHT 1080


#define MAX_WIDTH 1920
#define MAX_HEIGHT 1024

int sample_system_init(ImageAttributesConfigs configs);
int sample_system_exit();

int sample_framesource_streamon();
int sample_framesource_streamoff();

int sample_framesource_init();
int sample_framesource_exit();

int sample_jpeg_init();
int sample_encoder_exit();

int sample_res_deinit();
int sample_res_init();

/*图片属性设置*/
int sample_set_attributes_luminance(uint8_t luminance);
int sample_set_attributes_compression(uint8_t compression);
int sample_set_attributes_resolution(uint16_t width, uint16_t height);






#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __SAMPLE_COMMON_H__ */
