
#include "self_check.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/stat.h>
#include "cm_utils.h"
#include "cm_common.h"
#include "cm_conf.h"
#include <math.h>
#include "mpu_ctrl.h"
unsigned long long cm_tick_milli();

#define MPU_READ_ATTEMPTS 3
#define MPU_OPEN_ATTEMPTS 2
#define MPU_READ_INTERVAL_MS 100  // 读取间隔
#define MPU_VALUE_EPSILON 0.0001f // 用于浮点数比较

static bool is_ypr_valid(float ypr[3], float prev_ypr[3])
{
    // 检查数值是否为0
    if (fabsf(ypr[0]) < MPU_VALUE_EPSILON && 
        fabsf(ypr[1]) < MPU_VALUE_EPSILON && 
        fabsf(ypr[2]) < MPU_VALUE_EPSILON) {
        LOGD("WARNING: All YPR values are near zero");
        return false;
    }

    // 检查数值是否与上次相同（如果有上次的数据）
    if (prev_ypr != NULL) {
        if (fabsf(ypr[0] - prev_ypr[0]) < MPU_VALUE_EPSILON && 
            fabsf(ypr[1] - prev_ypr[1]) < MPU_VALUE_EPSILON && 
            fabsf(ypr[2] - prev_ypr[2]) < MPU_VALUE_EPSILON) {
            LOGD("WARNING: YPR values haven't changed from previous reading");
            return false;
        }
    }

    return true;
}

int gyro_self_check_start(SelfCheckProperty *checkProperty)
{
    LOGD(" ***Gyroscope Self Check Started*** ");

    if (!checkProperty) {
        LOGD("ERROR: Invalid checkProperty pointer");
        return -1;
    }

    // 初始化检查属性
    checkProperty->startTime = cm_gettime_milli();
    checkProperty->tickStart = cm_tick_milli();
    checkProperty->result = false;  // 初始设为失败
    
    // 尝试打开设备
    int ret = -1;
    for (int i = 0; i < MPU_OPEN_ATTEMPTS; i++) {
        ret = gyroscope_open();
        LOGD("Open MPU device attempt %d/%d: result=[%d]", 
             i + 1, MPU_OPEN_ATTEMPTS, ret);
        
        if (ret == 0) {
            LOGD("Successfully opened MPU device");
            break;
        }
        
        if (i < MPU_OPEN_ATTEMPTS - 1) {
            LOGD("Failed to open device, retrying in 1 second...");
            sleep(1);  
        }
    }
    if (ret < 0) {
        LOGD("ERROR: Failed to open MPU device after all attempts");
        goto exit;
    }

    /*等待陀螺仪就绪*/
    /*不需要等待陀螺仪校准，仅读取角速度数据*/
    // while(1)
    // {
    //     if (gyroscope_ready()!=0)
    //     {
    //         LOGD("wait for gyro ready");
    //         sleep(1);
    //     }
    //     else{
    //         break;
    //     }
    // }
    

        // 读取并验证YPR数据
    float gyro[3] = {0};
    float accel[3] = {0};
    /*先过滤前几次数据*/
    for(int i = 0; i < 10; i++)
    {
        gyroscope_read_gyro(gyro, accel);
        usleep(1000);
    }



    float prev_ypr[3] = {0};
    bool first_read = true;
    int valid_reads = 0;

    for (int i = 0; i < MPU_READ_ATTEMPTS; i++) {
        float ypr[3] = {0};
        /*仅读取角速度数据*/
        ret = gyroscope_read_gyro(gyro, accel);

        LOGD("Read attempt %d/%d - result [%d] YPR values: [%.2f, %.2f, %.2f]", 
             i + 1, MPU_READ_ATTEMPTS, ret, gyro[0], gyro[1], gyro[2]);

        if (ret >= 0) {
            if (is_ypr_valid(gyro, first_read ? NULL : prev_ypr)) {
                valid_reads++;
                memcpy(prev_ypr, ypr, sizeof(prev_ypr));
                first_read = false;
            }
        } else {
            LOGD("Failed to read YPR values");
        }

        // 在读取之间添加延时，使用 usleep
        if (i < MPU_READ_ATTEMPTS - 1) {
            usleep(MPU_READ_INTERVAL_MS * 1000);  // 转换毫秒为微秒
        }
    }

    // 判断最终结果
    checkProperty->result = (valid_reads >= 2);  // 至少需要2次有效读数
    LOGD("Valid readings: %d/%d", valid_reads, MPU_READ_ATTEMPTS);


exit:
    checkProperty->endTime = cm_gettime_milli();
    checkProperty->tickEnd = cm_tick_milli();
    
    unsigned long long duration = checkProperty->endTime - checkProperty->startTime;
    LOGD(" ***Gyroscope Self Check Completed*** ");
    LOGD("Duration: %llu ms", duration);
    LOGD("Final result: %s", checkProperty->result ? "SUCCESS" : "FAILED");
    LOGD(" ***End of Gyroscope Self Check*** ");
    
    return checkProperty->result ? 0 : -1;
}