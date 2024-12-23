#include "yq_mpu_impl_lsm6ds3trc.h"
#include "yq_mpu.h"
#include <stdio.h>      // for printf
#include <unistd.h>     // for sleep, usleep
#include <stdlib.h>     // for exit
const YQMpuImpl *impl_mpu_device = &yq_mpu_impl_lsm6ds3trc;
static YQMpuContext ctx;
int gyroscope_open(void)
{

    // uint64_t now = cm_tick_milli();
    int ret = impl_mpu_device->open(&ctx, impl_mpu_device->device);
    if (ret != 0)
    {
        return ret;
    }

    return 0;
}
int gyroscope_ready(void)
{
    return impl_mpu_device->ready(&ctx);
}
int gyroscope_close(void)
{
    return impl_mpu_device->close(&ctx);
}
int gyroscope_read(float ypr[3])
{
    // hmi_exception_report(); // ?

    if (impl_mpu_device->read_ypr(&ctx, ypr) <= 0)
    {

        return -1;
    }

    return 0;
}
#define version 1.11
int main(void)
{
    printf("version: %f\n", version);
    char gs_open_flag = 0;
    float ypr[3] = {0};
    while (1)
    {

        if (!gs_open_flag)
        {
            if (gyroscope_open() != 0)
            {
                printf("gyroscope open fail\n");
                gyroscope_close();
                sleep(30);
                continue;
            }
            else
            {
                gs_open_flag = 1;
            }
        }

        if (gyroscope_ready() != 0)
        {
            printf("wait gyroscope ready\n");
            sleep(1);
            continue;
        }
        if (gyroscope_read(ypr) == 0)
        {
            printf("ypr[0] = %f\n", ypr[0]);
            usleep(200000);
        }
    }
    return 0;
}