/**
 * @file cm_video_interface.h
 * @author jmdvirus
 */
#ifndef CM_VIDEO_INTERFACE_H
#define CM_VIDEO_INTERFACE_H

#include <unistd.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum
    {
        CM_VIDEO_FORMAT_NONE = 0, ///< None
        CM_VIDEO_FORMAT_H264,     ///< h264
        CM_VIDEO_FORMAT_H265,     ///< H265
        CM_VIDEO_FORMAT_YUYV,     ///< YUYV
        CM_VIDEO_FORMAT_JPEG,     ///< MJPEG
    } CM_VIDEO_FORMAT;

    /**
     * 视频数据流结构体
     */
    typedef struct
    {
        void  *start;
        size_t length;
    } CMVideoBuf;

    typedef struct
    {
        char dev[64];
        int  width;
        int  height;
        int  fps;
        char rc_mode[8];
        char codec[16];
        int  sformat;
        int  bitrate;
        int  channel;
    } CMCodecParam;

    /**
     * 上下文句柄对象
     */
    typedef struct
    {
        void *priv_data; // 内部使用，勿改，首次置 NULL
    } CMVideoContext;

    /**
     * control 控制命令列表
     */
    typedef enum
    {
        CM_VIDEO_CMD_NONE = 0,
        CM_VIDEO_CMD_SET_WIDTH,      // width
        CM_VIDEO_CMD_SET_HEIGHT,     // height
        CM_VIDEO_CMD_SET_BUF_TYPE,   // V4L2_BUF_TYPE_XXX (linux/videodev2.h)
        CM_VIDEO_CMD_SET_PIX_FORMAT, // V4L2_BUF_TYPE_XXX (linux/videodev2.h)
        CM_VIDEO_CMD_SET_FPS,
        CM_VIDEO_CMD_SET_CHANNEL,     
        CM_VIDEO_CMD_SET_KEY_FRAME,
    } CM_VIDEO_CMD;

    typedef struct
    {
        char *name; // 设置惟一识别码
        /**
         * 打开设备
         * @param ctx 上下文
         * @param dev 设备名
         */
        int (*open)(CMVideoContext *ctx, const char *dev);
        int (*close)(CMVideoContext *ctx);
        /**
         * 控制设备接口
         * @param ctx
         * @param cmd  命令见 CM_VIDEO_CMD
         * @param args
         */
        int (*ctrl)(CMVideoContext *ctx, int cmd, void *args);
        int (*start)(CMVideoContext *ctx);
        int (*stop)(CMVideoContext *ctx);
        /**
         * 读取数据，数据放到 buf 中，内部实现基于 poll,
         * 外部循环读即可，无需要等待
         * @param ctx
         * @param buf
         * @return : 1 读取到数据了
         *    0 没有出错，但没有读取到数据
         *    < 0 读取出错了
         */
        int (*read)(CMVideoContext *ctx, CMVideoBuf *buf);
    } CMVideoImpl;

    extern const CMVideoImpl cm_video_impl_t23;

#ifdef __cplusplus
}
#endif

#endif // CM_VIDEO_INTERFACE_H
