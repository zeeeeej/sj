
#include "yq_mpu.h"
#include <stdio.h>
static const YQMpuImpl *impl_mpu_device = &yq_mpu_impl_lsm6ds3trc;
static YQMpuContext ctx;
static int is_initialized = 0;
int gyroscope_open(void)
{

    // uint64_t now = cm_tick_milli();
        // 检查是否已经初始化
    if (is_initialized) {
        printf("Gyroscope already initialized\n");
        return 0;
    }
    int ret = impl_mpu_device->open(&ctx, impl_mpu_device->device);
    if (ret != 0)
    {
        return ret;
    }
    is_initialized = 1;
    return 0;
}

int gyroscope_ready(void)
{
    return impl_mpu_device->ready(&ctx);
}
int gyroscope_read_yqr(float ypr[3])
{

    if (impl_mpu_device->read_ypr(&ctx, ypr) <= 0)
    {

        return -1;
    }

    return 0;
}

int gyroscope_read_gyro(float gyro[3], float accel[3])
{
    if(impl_mpu_device->read_gyro(&ctx, gyro, accel)<=0)
    {
        return -1;
    }
    return 0;
}

int gyroscope_close(void)
{
    return impl_mpu_device->close(&ctx);
}