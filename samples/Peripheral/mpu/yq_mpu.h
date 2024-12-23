/**
 * @file yq_mpu.h
 * @author jmdvirus
 * @date 2021-06-30 17:00:33
 */

#ifndef JINDOU_MIDDLEWARE_YQ_MPU_H
#define JINDOU_MIDDLEWARE_YQ_MPU_H

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        void *priv_data;
    } YQMpuContext;

    typedef struct
    {
        const char *impl;
        const char *device;
        int  (*open)(YQMpuContext *ctx, const char *dev);
        int  (*close)(YQMpuContext *ctx);
        int  (*ready)(YQMpuContext *ctx);

        /**
         * 读取角度
         * @param ctx
         * @param ypr
         * @return
         *   @retval 0 结束 ，但没有数据
         *   @retval 1 有数据
         *   @retval <0 失败
         */
        int (*read_ypr)(YQMpuContext *ctx, float ypr[]);
        int (*read_gyro)(YQMpuContext *ctx, float gyro[], float accel[]);
    } YQMpuImpl;

    extern const YQMpuImpl yq_mpu_impl_lsm6ds3trc;


#ifdef __cplusplus
}
#endif

#endif // JINDOU_MIDDLEWARE_YQ_MPU_H
