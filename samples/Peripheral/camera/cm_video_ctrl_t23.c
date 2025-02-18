
#include <string.h>

#include "cm_video_ctrl.h"
#include "sample-common.h"
#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_framesource.h>
#include <imp/imp_encoder.h>
#include <sample-common.h>
#include <malloc.h>


#define TAG "[T23]"

typedef struct {
    CMVideoBuf buf;
    size_t buf_cap;
} CMVideoCtrlT23Private;

extern struct chn_conf wind_chn[];

static int cm_video_ctrl_open(CMVideoContext *ctx, const char *dev)
{
    int ret = wind_sample_system_init();
    if (ret < 0)
    {
        IMP_LOG_ERR(TAG, "system init failed ret = [%d]\n", ret);
        return -1;
    }
    ret = wind_sample_framesource_init();
    if (ret < 0)
    {
        IMP_LOG_ERR(TAG, "frame source init failed ret = [%d]\n", ret);
        return -2;
    }

    int i;
    for (i = 0; i < FS_CHN_NUM; i++) {
		if (wind_chn[i].enable) {
			ret = IMP_Encoder_CreateGroup(wind_chn[i].index);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_Encoder_CreateGroup(%d) error !\n", i);
				return -3;
			}
		}
	}

    ret = wind_sample_jpeg_init();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Encoder init failed ret = [%d]\n", ret);
		return -4;
	}

	//创建压缩率
	IMPEncoderJpegeQl pstJpegeQl;
	int compression = Get_compressibility();
	for (i = 0; i < FS_CHN_NUM; i++) 
	{
		if (wind_chn[i].enable) 
		{
			IMP_Encoder_GetJpegeQl(3+wind_chn[i].index, &pstJpegeQl);
			wind_MakeTables(compression, &(pstJpegeQl.qmem_table[0]), &(pstJpegeQl.qmem_table[64]));
			pstJpegeQl.user_ql_en = 1;
			IMP_Encoder_SetJpegeQl(3+wind_chn[i].index, &pstJpegeQl);
		}
	}


    for (i = 0; i < FS_CHN_NUM; i++) {
		if (wind_chn[i].enable) {
			ret = IMP_System_Bind(&wind_chn[i].framesource_chn, &wind_chn[i].imp_encoder);
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
		if (wind_chn[i].enable) {
			ret = IMP_System_UnBind(&wind_chn[i].framesource_chn, &wind_chn[i].imp_encoder);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "UnBind FrameSource channel%d and Encoder failed\n",i);
				return -1;
			}
		}
	}

	/* Step.c Encoder exit */
	ret = wind_sample_encoder_exit();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Encoder exit failed\n");
		return -2;
	}

	for (i = 0; i < FS_CHN_NUM; i++) {
		if (wind_chn[i].enable) {
			ret = IMP_Encoder_DestroyGroup(wind_chn[i].index);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_Encoder_CreateGroup(%d) error !\n", i);
				return -3;
			}
		}
	}

	/* Step.d FrameSource exit */
	ret = wind_sample_framesource_exit();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "FrameSource exit failed\n");
		return -4;
	}

	/* Step.e System exit */
	ret = wind_sample_system_exit();
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
    int ret = wind_sample_framesource_streamon();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "ImpStreamOn failed ret = [%d]\n", ret);
		return -2;
	}
    return 0;
}

static int cm_video_ctrl_stop(CMVideoContext *ctx)
{
    int ret = wind_sample_framesource_streamoff();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "FrameSource StreamOff failed = [%d]\n", ret);
		return -2;
	}
    return 0;
}

static int cm_video_ctrl_ctrl(CMVideoContext *ctx, int cmd, void *arg)
{
    return 0;
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
		if (wind_chn[i].enable) {
			ret = IMP_Encoder_StartRecvPic(3 + wind_chn[i].index);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_Encoder_StartRecvPic(%d) failed\n", 3 + wind_chn[i].index);
				return -1;
			}
            while (j++ < 3)
            {
				/* Polling JPEG Snap, set timeout as 1000msec */
				ret = IMP_Encoder_PollingStream(3 + wind_chn[i].index, 100);
				if (ret < 0) {
					IMP_LOG_ERR(TAG, "Polling stream timeout j = [%d]\n", j);
					continue;
				}

				IMPEncoderStream stream;
				/* Get JPEG Snap */
				ret = IMP_Encoder_GetStream(wind_chn[i].index + 3, &stream, 1);
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

				ret = IMP_Encoder_ReleaseStream(3 + wind_chn[i].index, &stream);
				break;
			}
        
			ret = IMP_Encoder_StopRecvPic(3 + wind_chn[i].index);
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
