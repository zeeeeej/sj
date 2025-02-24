/**
 * @file cm_video_interface.h
 * @author jmdvirus
 */
#ifndef CM_VIDEO_INTERFACE_H
#define CM_VIDEO_INTERFACE_H

#include <unistd.h>
#include <stdint.h>
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

    /**
     * 上下文句柄对象
     */
    typedef struct
    {
        void *priv_data; // 内部使用，勿改，首次置 NULL
    } CMVideoContext;

    /*控制*/
    typedef enum {
        PARAM_BRIGHTNESS = 1,   // 亮度参数
        PARAM_COMPRESSION,      // 压缩率参数
        PARAM_RESOLUTION,       // 分辨率参数（宽高）
        // 添加更多参数...
    } CMVideoParamID;

    typedef enum {
        CMD_SINGLE_PARAM,      // 设置单个参数
        CMD_MULTI_PARAMS,      // 设置多个参数
        CMD_OTHER_OPERATION,   // 其他操作类型
    } CMCommandType;

    typedef struct {
        CMVideoParamID param_id;
        union {
            uint8_t value;      // 亮度和压缩率
            struct {
                uint16_t width;  // 宽度
                uint16_t height; // 高度
            } resolution;        // 分辨率
        };
    } CMVideoParam;

    typedef struct {
        CMCommandType cmd_type;   // 命令类型
        CMVideoParam single_param; // 单参数模式
    } CMVideoCommand;

    /*video interface */
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
        int (*ctrl)(CMVideoContext *ctx, CMVideoCommand cmd, void *args);
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
