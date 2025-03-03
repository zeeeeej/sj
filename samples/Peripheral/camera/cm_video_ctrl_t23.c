#include <string.h>
#include <malloc.h>


#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_framesource.h>
#include <imp/imp_encoder.h>
#include "sample-common.h"

#include "cm_video_ctrl.h"
#include "cm_config.h"
#define TAG "[T23]"

typedef struct {
    CMVideoBuf buf;
    size_t buf_cap;
} CMVideoCtrlT23Private;

extern struct chn_conf chn[];
/*默认配置*/
#define DEFAULT_WIDTH 1920  /*分辨率*/
#define DEFAULT_HEIGHT 1024  /*分辨率*/
#define DEFAULT_LUMINANCE 128  /*亮度*/
#define DEFAULT_COMPRESSION 99  /*压缩率*/


/*从配置文件中读取配置信息*/
static int LoadImageAttributesConfigs(ImageAttributesConfigs* configs)
{
    int ret = 0;
    ret = Get_Camera_config(&configs->width, &configs->height);
    if(ret < 0)
    {
        IMP_LOG_ERR(TAG, "Get_Camera_config failed , use default value : %d, %d\n", DEFAULT_WIDTH, DEFAULT_HEIGHT);
        configs->width = DEFAULT_WIDTH;
        configs->height = DEFAULT_HEIGHT;
    }
    if(configs->width > MAX_WIDTH || configs->height > MAX_HEIGHT)
    {
        IMP_LOG_ERR(TAG, "Camera_config too large , use default value : %d, %d\n", DEFAULT_WIDTH, DEFAULT_HEIGHT);
        configs->width = DEFAULT_WIDTH;
        configs->height = DEFAULT_HEIGHT;
    }
    ret = Get_Luminance(&configs->luminance);
    if(ret < 0)
    {
        IMP_LOG_ERR(TAG, "Get_Luminance failed , use default value : %d\n", DEFAULT_LUMINANCE);
        configs->luminance = DEFAULT_LUMINANCE;
    }
    ret = Get_compressibility(&configs->compression);
    if(ret < 0)
    {
        IMP_LOG_ERR(TAG, "Get_compressibility failed , use default value : %d\n", DEFAULT_COMPRESSION);
        configs->compression = DEFAULT_COMPRESSION;
    }
    return 0;
}

static int cm_video_ctrl_open(CMVideoContext *ctx, const char *dev)
{
	ImageAttributesConfigs configs = {0};
    LoadImageAttributesConfigs(&configs);
    int ret = sample_system_init(configs);
    if (ret < 0)
    {
        IMP_LOG_ERR(TAG, "system init failed ret = [%d]\n", ret);
        return -1;
    }

    ret = sample_framesource_init();
    if (ret < 0)
    {
        IMP_LOG_ERR(TAG, "frame source init failed ret = [%d]\n", ret);
        return -2;
    }

    int i;
    for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			ret = IMP_Encoder_CreateGroup(chn[i].index);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_Encoder_CreateGroup(%d) error !\n", i);
				return -3;
			}
		}
	}

    ret = sample_jpeg_init();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Encoder init failed ret = [%d]\n", ret);
		return -4;
	}

    for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			ret = IMP_System_Bind(&chn[i].framesource_chn, &chn[i].imp_encoder);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "Bind FrameSource channel%d and Encoder failed ret = [%d]\n",i, ret);
				return -5;
			}
		}
	}
    CMVideoCtrlT23Private *in = (CMVideoCtrlT23Private*)calloc(1, sizeof(CMVideoCtrlT23Private));
    ctx->priv_data = in;

    return 0;
}

static int cm_video_ctrl_close(CMVideoContext *ctx)
{
    int i, ret = 0;
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
		return -2;
	}

	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			ret = IMP_Encoder_DestroyGroup(chn[i].index);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_Encoder_CreateGroup(%d) error !\n", i);
				return -3;
			}
		}
	}

	/* Step.d FrameSource exit */
	ret = sample_framesource_exit();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "FrameSource exit failed\n");
		return -4;
	}

	/* Step.e System exit */
	ret = sample_system_exit();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "wind_sample_system_exit() failed\n");
		return -5;
	}
    if (ctx->priv_data)
    {
        CMVideoCtrlT23Private *in = (CMVideoCtrlT23Private*)ctx->priv_data;
        if (in->buf.start)
        {
            free(in->buf.start);
        }
        free(ctx->priv_data);
        ctx->priv_data = NULL;
    }
    return 0;
}

static int cm_video_ctrl_start(CMVideoContext *ctx)
{
    int ret = sample_framesource_streamon();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "ImpStreamOn failed ret = [%d]\n", ret);
		return -2;
	}
    return 0;
}

static int cm_video_ctrl_stop(CMVideoContext *ctx)
{
    int ret = sample_framesource_streamoff();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "FrameSource StreamOff failed = [%d]\n", ret);
		return -2;
	}
    return 0;
}


static int cm_video_ctrl_ctrl(CMVideoContext *ctx, CMVideoCommand cmd, void *arg)
{    
    switch (cmd.cmd_type) {
    case CMD_SINGLE_PARAM:
        // 处理单个参数
        switch (cmd.single_param.param_id) {
        case PARAM_BRIGHTNESS:
            return sample_set_attributes_luminance(cmd.single_param.value);
        case PARAM_COMPRESSION:
            return sample_set_attributes_compression(cmd.single_param.value);
        case PARAM_RESOLUTION:
            // 设置分辨率
            return sample_set_attributes_resolution(cmd.single_param.resolution.width, cmd.single_param.resolution.height);
        default:
            printf("Unknown single parameter ID: %d\n", cmd.single_param.param_id);
            return -1;
        }
        break;

    default:
        printf("Unsupported command type: %d\n", cmd.cmd_type);
        return -1;
    }
}

static int cm_video_ctrl_read(CMVideoContext *ctx, CMVideoBuf *buf)
{
    CMVideoCtrlT23Private *in = (CMVideoCtrlT23Private*)ctx->priv_data;
    if (!in)
    {
        return -2;
    }
    int i, ret, j = 0;

	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			ret = IMP_Encoder_StartRecvPic(3 + chn[i].index);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_Encoder_StartRecvPic(%d) failed\n", 3 + chn[i].index);
				return -1;
			}
            while (j++ < 3)
            {
				/* Polling JPEG Snap, set timeout as 1000msec */
				ret = IMP_Encoder_PollingStream(3 + chn[i].index, 100);
				if (ret < 0) {
					IMP_LOG_ERR(TAG, "Polling stream timeout j = [%d]\n", j);
					continue;
				}

				IMPEncoderStream stream;
				/* Get JPEG Snap */
				ret = IMP_Encoder_GetStream(chn[i].index + 3, &stream, 1);
				if (ret < 0) {
					IMP_LOG_ERR(TAG, "IMP_Encoder_GetStream() failed\n");
					return -2;
				}
                if (stream.packCount == 0)
                {
                    IMP_LOG_ERR(TAG, "stream.packCount == 0\n");
                    continue;
                }

                if (in->buf.start == NULL)
                {
                    in->buf.start = malloc(stream.pack[0].length);
                    in->buf.length = stream.pack[0].length;
                }
                int curLength = 0;
                int needLength = 0;
				int z = 0;
                for (z = 0; z < stream.packCount; z++)
                {
                    needLength += stream.pack[z].length;
                    if (in->buf_cap < needLength)
                    {
                        in->buf.start = realloc(in->buf.start, needLength);
                        in->buf_cap = needLength;
                    }
                    memcpy((uint8_t*)in->buf.start + curLength, (void*)stream.pack[z].virAddr, stream.pack[z].length);
                    curLength += stream.pack[z].length;
                }
                buf->length = curLength;
                buf->start = in->buf.start;

				ret = IMP_Encoder_ReleaseStream(3 + chn[i].index, &stream);
				break;
			}
        
			ret = IMP_Encoder_StopRecvPic(3 + chn[i].index);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_Encoder_StopRecvPic() failed\n");
				return -3;
			}
		}
    }
	return 0;
}

const CMVideoImpl cm_video_impl_t23 =
{
    .name = "t23",
    .open = cm_video_ctrl_open,
    .close = cm_video_ctrl_close,
    .start = cm_video_ctrl_start,
    .stop = cm_video_ctrl_stop,
    .read = cm_video_ctrl_read,
    .ctrl = cm_video_ctrl_ctrl,
};
