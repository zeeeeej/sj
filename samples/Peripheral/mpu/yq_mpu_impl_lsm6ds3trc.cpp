#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <vector>
#include <stdio.h>
#include <fcntl.h>
#include <iostream>
#include <cstring>
#include <thread>
#include <iomanip>
#include <cmath>
#include <chrono>
#include <sys/resource.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <fstream>
#include "cm_common.h"
// #include "jemalloc/jemalloc.h"
// #include "knlog.h"
#include "yq_mpu.h"
#include "cm_utils.h"
// #include "shell_command.h"
    typedef enum
    {
        YQ_ERROR_SUCCESS = 0,   // 一般指动作结束， 没有产生错误
        YQ_ERROR_DONE = 1,      // 一般指读取操作成功，有数据
        YQ_ERROR_NO_MEM = -100, // -99 -98
        YQ_ERROR_ARGS,
        YQ_ERROR_NO_PERMISSION,
        YQ_ERROR_NOT_EXIST,
        YQ_ERROR_HAS_EXIST,
        YQ_ERROR_FILE_OPEN,
        YQ_ERROR_FILE_CLOSE,
        YQ_ERROR_FILE_READ,
        YQ_ERROR_FILE_WRITE,
        YQ_ERROR_NOT_INITED,
        YQ_ERROR_HAS_INITED,
        YQ_ERROR_NOT_STARTED,
        YQ_ERROR_HAS_STARTED,

        YQ_ERROR_CUSTOM_NEXT = -1000,
    } YQError;
class Mpu_Lsm6ds3trc //: public CommandRegistry
{
  private:
    const char *device;
    int   i2c_fd;
    int   i2c_addr;

    bool        i2c_error = false;
    bool        is_exited = false;
    std::thread thread_;

    float q0 = 1, q1 = 0, q2 = 0, q3 = 0;
    float exInt = 0, eyInt = 0, ezInt = 0;
    float Yaw = 0, Pitch = 0, Roll = 0;
    float lastProcTime = 0.0;

    constexpr const static float Kp    = 10.0f;
    constexpr const static float Ki    = 0.001f;
    constexpr const static float scale = 57.2957795131f;
    constexpr const static float YAW_SCALE = -10.4142;

    std::vector<float> acc             = {0, 0, 0};
    std::vector<float> gyr             = {0, 0, 0};
    std::vector<float> gyr_dev         = {0, 0, 0};
    std::vector<float> gyr_cali        = {0, 0, 0};
    std::vector<float> acc_cali        = {0, 0, 0};
    bool               calibrate       = false;
    int                calibrate_count = 0;

    bool debug_flag = false;

  public:
    Mpu_Lsm6ds3trc(const char *device, int i2c_addr) //: CommandRegistry()
    {
        this->device   = device;
        this->i2c_addr = i2c_addr;
        //registeCmd("MpuStatus", std::bind(&Mpu_Lsm6ds3trc::MpuStatus, this, std::placeholders::_1));
    }

    ~Mpu_Lsm6ds3trc()
    {
        is_exited = true;
        // Wait for the thread to finish
        if (thread_.joinable())
        {
            thread_.join();
        }
        LOGD("Mpu_Lsm6ds3trc destroyed!!!");
    }

#if 0
    void MpuStatus(const std::vector<std::string> &args)
    {
        int loop = 1;
        if (args.size() > 1)
        {
            auto operation = parseArgument<std::string>(args[1]);
            if (operation == "debug")
            {
                if (args.size() > 2)
                {
                    auto param = parseArgument<std::string>(args[2]);
                    if (param == "on")
                    {
                        std::cout << "set debug on" << std::endl;
                        debug_flag = true;
                    }
                    else if (param == "off")
                    {
                        std::cout << "set debug off" << std::endl;
                        debug_flag = false;
                    }
                }
            }
            else if (operation == "loop")
            {
                if (args.size() > 2)
                {
                    loop = parseArgument<int>(args[2]);
                }
            }
        }

        for (int icnt = 0; icnt < loop; icnt++)
        {
            std::cout << "Mpu status" << std::endl;
            std::cout << std::string(32, '-') << std::endl;
            std::cout << "calibrate: " << calibrate << std::endl;
            std::cout << "gyr_cali : "
                      << "[" << gyr_cali[0] << ", " << gyr_cali[1] << ", " << gyr_cali[2] << "]" << std::endl;
            std::cout << "gyr_dev  : "
                      << "[" << gyr_dev[0] << ", " << gyr_dev[1] << ", " << gyr_dev[2] << "]" << std::endl;
            std::cout << "gyr_used : "
                      << "[" << gyr[0] << ", " << gyr[1] << ", " << gyr[2] << "]" << std::endl;
            std::cout << "acc_value: "
                      << "[" << acc[0] << ", " << acc[1] << ", " << acc[2] << "]" << std::endl;
            std::cout << "rpy_value: "
                      << "[" << Roll << ", " << Pitch << ", " << Yaw*YAW_SCALE<< "]" << std::endl;
            if (icnt < loop - 1)
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
#endif

    bool Lsm6ds3trc_Init(void)
    {
        i2c_fd = open(device, O_RDWR);
        if (i2c_fd < 0)
        {
            LOGD("Failed to open the i2c device %s", device);
            return false;
        }
        if (ioctl(i2c_fd, I2C_SLAVE, i2c_addr) < 0)
        {
            LOGD("Failed to acquire bus access and/or talk to slave");
            return false;
        }

        uint8_t              buf[] = {LSM6DS3TRC_WHO_AM_I};
        std::vector<uint8_t> data  = Lsm6ds3trc_read(buf);
        if (data[0] != LSM6DS3TRC_WHO_AM_I_RESP)
        {
            LOGD("LSM6DS3TRC_WHO_AM_I_RESP = 0x%x, not equal 0x%x", data[0], LSM6DS3TRC_WHO_AM_I_RESP);
            return false;
        }
        Lsm6ds3trc_Reset();
        Lsm6ds3trc_Set_BDU(true);
        Lsm6ds3trc_Set_Accelerometer_Rate(LSM6DS3TRC_ACC_RATE_833HZ);
        Lsm6ds3trc_Set_Gyroscope_Rate(LSM6DS3TRC_GYR_RATE_833HZ);
        Lsm6ds3trc_Set_Accelerometer_Fullscale(LSM6DS3TRC_ACC_FSXL_2G);
        Lsm6ds3trc_Set_Gyroscope_Fullscale(LSM6DS3TRC_GYR_FSG_2000);
        Lsm6ds3trc_Set_Accelerometer_Bandwidth(LSM6DS3TRC_ACC_BW0XL_400HZ, LSM6DS3TRC_ACC_LOW_PASS_ODR_100);
        Lsm6ds3trc_Set_Register7(LSM6DS3TRC_CTRL7_G_HM_MODE_DISABLE | LSM6DS3TRC_CTRL7_G_HPM_260MHZ);
        Lsm6ds3trc_Set_Register6(LSM6DS3TRC_CTRL6_C_FTYPE_1);
        Lsm6ds3trc_Set_Register4(LSM6DS3TRC_CTRL4_LPF1_SELG_ENABLE);

        thread_ = std::thread(&Mpu_Lsm6ds3trc::GetDataThread, this);
        LOGD("Lsm6ds3trc_Init success");
        return true;
    }

    int  glbprint = 0;
    void IMUupdate(float gx, float gy, float gz, float ax, float ay, float az)
    {
        /*
        float norm;
        float vx, vy, vz;
        float ex, ey, ez;

        norm = sqrt(ax * ax + ay * ay + az * az);
        ax   = ax / norm;
        ay   = ay / norm;
        az   = az / norm;

        // 估计方向的重力
        vx = 2 * (q1 * q3 - q0 * q2);
        vy = 2 * (q0 * q1 + q2 * q3);
        vz = q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3;

        ex = (ay * vz - az * vy);
        ey = (az * vx - ax * vz);
        ez = (ax * vy - ay * vx);

        // 积分误差比例积分增益,计算陀螺仪测量的重力向量与估计方向的重力向量之间的误差
        exInt = exInt + ex * Ki;
        eyInt = eyInt + ey * Ki;
        ezInt = ezInt + ez * Ki;

        gx = gx + Kp * ex + exInt;
        gy = gy + Kp * ey + eyInt;
        gz = gz + Kp * ez + ezInt;

        float curProcTime   = std::chrono::steady_clock::now().time_since_epoch().count() / 1000000000.0f;
        float timeDelta     = curProcTime - lastProcTime;
        float halftimeDelta = timeDelta * 0.5;
        lastProcTime        = curProcTime;

        q0 = q0 + (-q1 * gx - q2 * gy - q3 * gz) * halftimeDelta;
        q1 = q1 + (q0 * gx + q2 * gz - q3 * gy) * halftimeDelta;
        q2 = q2 + (q0 * gy - q1 * gz + q3 * gx) * halftimeDelta;
        q3 = q3 + (q0 * gz + q1 * gy - q2 * gx) * halftimeDelta;

        // 归一化四元数
        norm = sqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
        q0   = q0 / norm;
        q1   = q1 / norm;
        q2   = q2 / norm;
        q3   = q3 / norm;

        Pitch = asin(2 * q2 * q3 + 2 * q0 * q1) * scale;
        Roll  = atan2(-2 * q1 * q3 + 2 * q0 * q2, q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3) * scale;
        // Yaw = atan2(2*(q1*q2 - q0*q3),q0*q0-q1*q1+q2*q2-q3*q3) * 57.3; //飘移太大
        */

        float curProcTime   = std::chrono::steady_clock::now().time_since_epoch().count() / 1000000000.0f;
        float timeDelta     = curProcTime - lastProcTime;
        float halftimeDelta = timeDelta * 0.5;
        lastProcTime        = curProcTime;

        if (fabs(gx * scale) > 1.0f) {
            Roll += gx * timeDelta;
        }

        if (fabs(gy * scale) > 1.0f) {
            Pitch += gy * timeDelta;
        }

        if (fabs(gz * scale) > 1.0f) {
            // printf(">0.01f gz : %f     timeDelta : %f     add : %f\n",gz,timeDelta,gz*timeDelta);
            Yaw += gz * timeDelta;
        }
        else
        {
            // printf("<0.01f gz : %f     timeDelta : %f     add : %f\n",gz,timeDelta,gz*timeDelta);
        }

        if (debug_flag) {
            uint64_t timecount = std::chrono::duration_cast<std::chrono::milliseconds>(
                                     std::chrono::system_clock::now().time_since_epoch()).count();
            std::cout << std::dec << "[" << timecount / 1000 << "." << std::setw(3) << timecount % 1000 
                      << " gyr: [" << std::fixed << std::setprecision(3) << std::setw(8) << gyr_cali[0] << ", " << std::setw(8) << gyr_dev[0] << ", " << std::setw(8) << gyr[0] << "]"
                      << "] rpy: [" << std::fixed << std::setprecision(3) << std::setw(8) << Roll << ", " << std::setw(8) << Pitch << ", " << std::setw(8) << Yaw*YAW_SCALE << "] "
                      << "u_g: [" << std::fixed << std::setprecision(3) << std::setw(8) << gx * scale << ", " << std::setw(8) << gy * scale << ", " << std::setw(8) << gz * scale << "] " << std::endl;
        }
    }

    void GetDataThread(void)
    {
        
        std::ofstream log_file;
    log_file.open("/system/gyr0_avg_log.txt", std::ios::app);
    if (!log_file.is_open()) {
            LOGD("Failed to open log file");
        }
        static float test_max = 0;
        static float sum_gyr0 = 0;  // 累加 gyr[0] 的值
        static int count_gyr0 = 0;  // 计数器
        float temp;
        LOGD("Lsm6ds3trc_GetDataThread start 2");
        prctl(PR_SET_NAME, "mpudata");
        while (!is_exited)
        {
            uint8_t status = Lsm6ds3trc_Get_Status();
            if (status & 0x03)
            {
                acc     = Lsm6ds3trc_Get_Acceleration(LSM6DS3TRC_ACC_FSXL_2G);
                gyr     = Lsm6ds3trc_Get_Gyroscope(LSM6DS3TRC_GYR_FSG_2000);
                gyr_dev = gyr;
            }
            else
            {
                if (status & 0x04)
                {
                    temp = Lsm6ds3trc_Get_Temperature();
                    std::cout << "TEMPERATURE: " << std::fixed << std::setprecision(3) << std::setw(7) << temp << "°C"
                              << std::endl;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }

            if (!calibrate)
            {
                if (calibrate_count == 0)
                    LOGD("mpu calibrate start....");
                gyr_cali[0] += gyr[0];
                gyr_cali[1] += gyr[1];
                gyr_cali[2] += gyr[2];

                acc_cali[2] += acc[2];
                if (calibrate_count++ < 3000)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    continue;
                }

                gyr_cali[0] = gyr_cali[0] / calibrate_count;
                gyr_cali[1] = gyr_cali[1] / calibrate_count;
                gyr_cali[2] = gyr_cali[2] / calibrate_count;

                acc_cali[2] = acc_cali[2] / calibrate_count;
                calibrate   = true;
                LOGD("mpu calibrate done, data=[%f, %f, %f]", gyr_cali[0], gyr_cali[1], gyr_cali[2]);
                lastProcTime = std::chrono::steady_clock::now().time_since_epoch().count() / 1000000000.0f;
            }

            gyr[0] -= gyr_cali[0];
            gyr[1] -= gyr_cali[1];
            gyr[2] -= gyr_cali[2];

            if (std::abs(gyr[0]) < 1000)
            {
                gyr[0] = 0;
            }
            
            if (std::abs(gyr[1]) < 1000)
                gyr[1] = 0;
            if (std::abs(gyr[2]) < 1000)
                gyr[2] = 0;
            IMUupdate(gyr[2] / 1000, gyr[1] / 1000, -gyr[0] / 1000, acc[2] / 1000, acc[1] / 1000, acc[0] / 1000);
            std::this_thread::sleep_for(std::chrono::milliseconds(6));
        }
        // 关闭文件
        if (log_file.is_open()) {
            log_file.close();
        }
        LOGD("mpu thread exit!!!");
    }

    bool Lsm6ds3trc_Ready(void)
    {
        return calibrate;
    }

    bool Lsm6ds3trc_Get_YawPitchRoll(float *ypr)
    {
        ypr[0] = Yaw*YAW_SCALE;
        ypr[1] = Pitch;
        ypr[2] = Roll;
        return i2c_error;
    }

    bool Lsm6ds3trc_Get_gyro(float *gyro)
    {
        gyro[0] = gyr_dev[0];
        gyro[1] = gyr_dev[1];
        gyro[2] = gyr_dev[2];
        return i2c_error;
    }


    float Lsm6ds3trc_Get_Temperature()
    {
        uint8_t              buf[] = {LSM6DS3TRC_OUT_TEMP_L};
        std::vector<uint8_t> data  = Lsm6ds3trc_read(buf, 2);
        int16_t              temp  = (data[1] << 8) | data[0];
        return temp / 256.0 + 25.0;
    }

    std::vector<float> Lsm6ds3trc_Get_Gyroscope(int fsg)
    {
        uint8_t              buf[] = {LSM6DS3TRC_OUTX_L_G};
        std::vector<uint8_t> data  = Lsm6ds3trc_read(buf, 6);
        std::vector<int16_t> gry   = {(int16_t)((data[1] << 8) | data[0]), (int16_t)((data[3] << 8) | data[2]),
                                    (int16_t)((data[5] << 8) | data[4])};
        std::vector<float>   gry_float(3, 0.0f);

        float scale_factor = 0.0f;
        switch (fsg)
        {
        case LSM6DS3TRC_GYR_FSG_245:
            scale_factor = 8.75f;
            break;
        case LSM6DS3TRC_GYR_FSG_500:
            scale_factor = 17.50f;
            break;
        case LSM6DS3TRC_GYR_FSG_1000:
            scale_factor = 35.00f;
            break;
        case LSM6DS3TRC_GYR_FSG_2000:
            scale_factor = 70.00f;
            break;
        }

        for (int i = 0; i < 3; ++i)
        {
            gry_float[i] = gry[i] * scale_factor;
        }

        return gry_float;
    }

    std::vector<float> Lsm6ds3trc_Get_Acceleration(int fsxl)
    {
        uint8_t              buf[] = {LSM6DS3TRC_OUTX_L_XL};
        std::vector<uint8_t> data  = Lsm6ds3trc_read(buf, 6);
        std::vector<int16_t> acc   = {(int16_t)((data[1] << 8) | data[0]), (int16_t)((data[3] << 8) | data[2]),
                                    (int16_t)((data[5] << 8) | data[4])};
        std::vector<float>   acc_float(3, 0.0f);

        float scale_factor = 0.0f;
        switch (fsxl)
        {
        case LSM6DS3TRC_ACC_FSXL_2G:
            scale_factor = 0.061f;
            break;
        case LSM6DS3TRC_ACC_FSXL_16G:
            scale_factor = 0.488f;
            break;
        case LSM6DS3TRC_ACC_FSXL_4G:
            scale_factor = 0.122f;
            break;
        case LSM6DS3TRC_ACC_FSXL_8G:
            scale_factor = 0.244f;
            break;
        }

        for (int i = 0; i < 3; ++i)
        {
            acc_float[i] = acc[i] * scale_factor;
        }

        return acc_float;
    }

    uint8_t Lsm6ds3trc_Get_Status()
    {
        uint8_t              buf[] = {LSM6DS3TRC_STATUS_REG};
        std::vector<uint8_t> data  = Lsm6ds3trc_read(buf);
        return data[0];
    }

  private:
    std::vector<uint8_t> Lsm6ds3trc_read(uint8_t *buf, int length = 1)
    {
        std::vector<uint8_t> data(length, 0);
        if (write(i2c_fd, buf, 1) != 1)
        {
            i2c_error = true;
            LOGD("Failed to write to the i2c bus");
            return data;
        }
        if (read(i2c_fd, data.data(), length) != length)
        {
            i2c_error = true;
            LOGD("Failed to read from the i2c bus");
            return data;
        }

        i2c_error = false;
        return data;
    }

    void Lsm6ds3trc_write(uint8_t *buf, int length = 2)
    {
        if (write(i2c_fd, buf, length) != length)
        {
            i2c_error = true;
            LOGD("Failed to write to the i2c bus");
        }
        i2c_error = false;
    }

    void Lsm6ds3trc_Set_Register4(uint8_t reg4)
    {
        uint8_t              buf[]  = {LSM6DS3TRC_CTRL4_C};
        std::vector<uint8_t> data   = Lsm6ds3trc_read(buf);
        uint8_t              buf2[] = {LSM6DS3TRC_CTRL4_C, static_cast<uint8_t>(data[0] | reg4)};
        Lsm6ds3trc_write(buf2);
    }

    void Lsm6ds3trc_Set_Register5(uint8_t reg5)
    {
        uint8_t              buf[]  = {LSM6DS3TRC_CTRL5_C};
        std::vector<uint8_t> data   = Lsm6ds3trc_read(buf);
        uint8_t              buf2[] = {LSM6DS3TRC_CTRL5_C, static_cast<uint8_t>(data[0] | reg5)};
        Lsm6ds3trc_write(buf2);
    }

    void Lsm6ds3trc_Set_Register6(uint8_t reg6)
    {
        uint8_t              buf[]  = {LSM6DS3TRC_CTRL6_C};
        std::vector<uint8_t> data   = Lsm6ds3trc_read(buf);
        uint8_t              buf2[] = {LSM6DS3TRC_CTRL6_C, static_cast<uint8_t>(data[0] | reg6)};
        Lsm6ds3trc_write(buf2);
    }

    void Lsm6ds3trc_Set_Register7(uint8_t reg7)
    {
        uint8_t              buf[]  = {LSM6DS3TRC_CTRL7_G};
        std::vector<uint8_t> data   = Lsm6ds3trc_read(buf);
        uint8_t              buf2[] = {LSM6DS3TRC_CTRL7_G, static_cast<uint8_t>(data[0] | reg7)};
        Lsm6ds3trc_write(buf2);
    }

    void Lsm6ds3trc_Set_Accelerometer_Bandwidth(uint8_t BW0XL, uint8_t ODR)
    {
        uint8_t              buf[]  = {LSM6DS3TRC_CTRL1_XL};
        std::vector<uint8_t> data   = Lsm6ds3trc_read(buf);
        uint8_t              buf2[] = {LSM6DS3TRC_CTRL1_XL, static_cast<uint8_t>(data[0] | BW0XL)};
        Lsm6ds3trc_write(buf2);
        buf[0]  = LSM6DS3TRC_CTRL8_XL;
        data    = Lsm6ds3trc_read(buf);
        buf2[0] = LSM6DS3TRC_CTRL8_XL;
        buf2[1] = static_cast<uint8_t>(data[0] | ODR);
        Lsm6ds3trc_write(buf2);
    }

    void Lsm6ds3trc_Set_Gyroscope_Fullscale(uint8_t value)
    {
        uint8_t              buf[]  = {LSM6DS3TRC_CTRL2_G};
        std::vector<uint8_t> data   = Lsm6ds3trc_read(buf);
        uint8_t              buf2[] = {LSM6DS3TRC_CTRL2_G, static_cast<uint8_t>(data[0] | value)};
        Lsm6ds3trc_write(buf2);
    }

    void Lsm6ds3trc_Set_Accelerometer_Fullscale(uint8_t value)
    {
        uint8_t              buf[]  = {LSM6DS3TRC_CTRL1_XL};
        std::vector<uint8_t> data   = Lsm6ds3trc_read(buf);
        uint8_t              buf2[] = {LSM6DS3TRC_CTRL1_XL, static_cast<uint8_t>(data[0] | value)};
        Lsm6ds3trc_write(buf2);
    }

    void Lsm6ds3trc_Set_Gyroscope_Rate(uint8_t rate)
    {
        uint8_t              buf[]  = {LSM6DS3TRC_CTRL2_G};
        std::vector<uint8_t> data   = Lsm6ds3trc_read(buf);
        uint8_t              buf2[] = {LSM6DS3TRC_CTRL2_G, static_cast<uint8_t>(data[0] | rate)};
        Lsm6ds3trc_write(buf2);
    }

    void Lsm6ds3trc_Set_Accelerometer_Rate(uint8_t rate)
    {
        uint8_t              buf[]  = {LSM6DS3TRC_CTRL1_XL};
        std::vector<uint8_t> data   = Lsm6ds3trc_read(buf);
        uint8_t              buf2[] = {LSM6DS3TRC_CTRL1_XL, static_cast<uint8_t>(data[0] | rate)};
        Lsm6ds3trc_write(buf2);
    }

    void Lsm6ds3trc_Set_BDU(bool flag)
    {
        uint8_t              buf[] = {LSM6DS3TRC_CTRL3_C};
        std::vector<uint8_t> data  = Lsm6ds3trc_read(buf);
        uint8_t buf2[] = {LSM6DS3TRC_CTRL3_C, static_cast<uint8_t>(flag ? (data[0] | 0x40) : (data[0] & 0xbf))};
        Lsm6ds3trc_write(buf2);
    }

    void Lsm6ds3trc_Reset()
    {
        uint8_t buf[] = {LSM6DS3TRC_CTRL3_C, 0x80};
        Lsm6ds3trc_write(buf);
        usleep(15000);
        uint8_t              buf2[] = {LSM6DS3TRC_CTRL3_C};
        std::vector<uint8_t> data   = Lsm6ds3trc_read(buf2);
        data[0] |= 0x01;
        buf[1] = data[0];
        Lsm6ds3trc_write(buf);

        /*
        std::vector<uint8_t> status(1, 1)
        while (status[0]&0x01)
        {
            uint8_t buf[] = {LSM6DS3TRC_CTRL3_C}
            data = Lsm6ds3trc_read(buf);
        }
        */
    }

  private:
    const static uint8_t LSM6DS3TRC_FUNC_CFG_ACCESS        = 0x01;
    const static uint8_t LSM6DS3TRC_SENSOR_SYNC_TIME_FRAME = 0x02;
    const static uint8_t LSM6DS3TRC_FIFO_CTRL1             = 0x06;
    const static uint8_t LSM6DS3TRC_FIFO_CTRL2             = 0x07;
    const static uint8_t LSM6DS3TRC_FIFO_CTRL3             = 0x08;
    const static uint8_t LSM6DS3TRC_FIFO_CTRL4             = 0x09;
    const static uint8_t LSM6DS3TRC_FIFO_CTRL5             = 0x0A;
    const static uint8_t LSM6DS3TRC_ORIENT_CFG_G           = 0x0B;
    const static uint8_t LSM6DS3TRC_INT1_CTRL              = 0x0D;
    const static uint8_t LSM6DS3TRC_INT2_CTRL              = 0x0E;
    const static uint8_t LSM6DS3TRC_WHO_AM_I               = 0x0F;
    const static uint8_t LSM6DS3TRC_CTRL1_XL               = 0x10;
    const static uint8_t LSM6DS3TRC_CTRL2_G                = 0x11;
    const static uint8_t LSM6DS3TRC_CTRL3_C                = 0x12;
    const static uint8_t LSM6DS3TRC_CTRL4_C                = 0x13;
    const static uint8_t LSM6DS3TRC_CTRL5_C                = 0x14;
    const static uint8_t LSM6DS3TRC_CTRL6_C                = 0x15;
    const static uint8_t LSM6DS3TRC_CTRL7_G                = 0x16;
    const static uint8_t LSM6DS3TRC_CTRL8_XL               = 0x17;
    const static uint8_t LSM6DS3TRC_CTRL9_XL               = 0x18;
    const static uint8_t LSM6DS3TRC_CTRL10_C               = 0x19;
    const static uint8_t LSM6DS3TRC_MASTER_CONFIG          = 0x1A;
    const static uint8_t LSM6DS3TRC_WAKE_UP_SRC            = 0x1B;
    const static uint8_t LSM6DS3TRC_TAP_SRC                = 0x1C;
    const static uint8_t LSM6DS3TRC_D6D_SRC                = 0x1D;
    const static uint8_t LSM6DS3TRC_STATUS_REG             = 0x1E;
    const static uint8_t LSM6DS3TRC_OUT_TEMP_L             = 0x20;
    const static uint8_t LSM6DS3TRC_OUT_TEMP_H             = 0x21;
    const static uint8_t LSM6DS3TRC_OUTX_L_G               = 0x22;
    const static uint8_t LSM6DS3TRC_OUTX_H_G               = 0x23;
    const static uint8_t LSM6DS3TRC_OUTY_L_G               = 0x24;
    const static uint8_t LSM6DS3TRC_OUTY_H_G               = 0x25;
    const static uint8_t LSM6DS3TRC_OUTZ_L_G               = 0x26;
    const static uint8_t LSM6DS3TRC_OUTZ_H_G               = 0x27;
    const static uint8_t LSM6DS3TRC_OUTX_L_XL              = 0x28;
    const static uint8_t LSM6DS3TRC_OUTX_H_XL              = 0x29;
    const static uint8_t LSM6DS3TRC_OUTY_L_XL              = 0x2A;
    const static uint8_t LSM6DS3TRC_OUTY_H_XL              = 0x2B;
    const static uint8_t LSM6DS3TRC_OUTZ_L_XL              = 0x2C;
    const static uint8_t LSM6DS3TRC_OUTZ_H_XL              = 0x2D;
    const static uint8_t LSM6DS3TRC_SENSORHUB1_REG         = 0x2E;
    const static uint8_t LSM6DS3TRC_SENSORHUB2_REG         = 0x2F;
    const static uint8_t LSM6DS3TRC_SENSORHUB3_REG         = 0x30;
    const static uint8_t LSM6DS3TRC_SENSORHUB4_REG         = 0x31;
    const static uint8_t LSM6DS3TRC_SENSORHUB5_REG         = 0x32;
    const static uint8_t LSM6DS3TRC_SENSORHUB6_REG         = 0x33;
    const static uint8_t LSM6DS3TRC_SENSORHUB7_REG         = 0x34;
    const static uint8_t LSM6DS3TRC_SENSORHUB8_REG         = 0x35;
    const static uint8_t LSM6DS3TRC_SENSORHUB9_REG         = 0x36;
    const static uint8_t LSM6DS3TRC_SENSORHUB10_REG        = 0x37;
    const static uint8_t LSM6DS3TRC_SENSORHUB11_REG        = 0x38;
    const static uint8_t LSM6DS3TRC_SENSORHUB12_REG        = 0x39;
    const static uint8_t LSM6DS3TRC_FIFO_STATUS1           = 0x3A;
    const static uint8_t LSM6DS3TRC_FIFO_STATUS2           = 0x3B;
    const static uint8_t LSM6DS3TRC_FIFO_STATUS3           = 0x3C;
    const static uint8_t LSM6DS3TRC_FIFO_STATUS4           = 0x3D;
    const static uint8_t LSM6DS3TRC_FIFO_DATA_OUT_L        = 0x3E;
    const static uint8_t LSM6DS3TRC_FIFO_DATA_OUT_H        = 0x3F;
    const static uint8_t LSM6DS3TRC_TIMESTAMP0_REG         = 0x40;
    const static uint8_t LSM6DS3TRC_TIMESTAMP1_REG         = 0x41;
    const static uint8_t LSM6DS3TRC_TIMESTAMP2_REG         = 0x42;
    const static uint8_t LSM6DS3TRC_STEP_TIMESTAMP_L       = 0x49;
    const static uint8_t LSM6DS3TRC_STEP_TIMESTAMP_H       = 0x4A;
    const static uint8_t LSM6DS3TRC_STEP_COUNTER_L         = 0x4B;
    const static uint8_t LSM6DS3TRC_STEP_COUNTER_H         = 0x4C;
    const static uint8_t LSM6DS3TRC_SENSORHUB13_REG        = 0x4D;
    const static uint8_t LSM6DS3TRC_SENSORHUB14_REG        = 0x4E;
    const static uint8_t LSM6DS3TRC_SENSORHUB15_REG        = 0x4F;
    const static uint8_t LSM6DS3TRC_SENSORHUB16_REG        = 0x50;
    const static uint8_t LSM6DS3TRC_SENSORHUB17_REG        = 0x51;
    const static uint8_t LSM6DS3TRC_SENSORHUB18_REG        = 0x52;
    const static uint8_t LSM6DS3TRC_FUNC_SRC               = 0x53;
    const static uint8_t LSM6DS3TRC_TAP_CFG                = 0x58;
    const static uint8_t LSM6DS3TRC_TAP_THS_6D             = 0x59;
    const static uint8_t LSM6DS3TRC_INT_DUR2               = 0x5A;
    const static uint8_t LSM6DS3TRC_WAKE_UP_THS            = 0x5B;
    const static uint8_t LSM6DS3TRC_WAKE_UP_DUR            = 0x5C;
    const static uint8_t LSM6DS3TRC_FREE_FALL              = 0x5D;
    const static uint8_t LSM6DS3TRC_MD1_CFG                = 0x5E;
    const static uint8_t LSM6DS3TRC_MD2_CFG                = 0x5F;
    const static uint8_t LSM6DS3TRC_OUT_MAG_RAW_X_L        = 0x66;
    const static uint8_t LSM6DS3TRC_OUT_MAG_RAW_X_H        = 0x67;
    const static uint8_t LSM6DS3TRC_OUT_MAG_RAW_Y_L        = 0x68;
    const static uint8_t LSM6DS3TRC_OUT_MAG_RAW_Y_H        = 0x69;
    const static uint8_t LSM6DS3TRC_OUT_MAG_RAW_Z_L        = 0x6A;
    const static uint8_t LSM6DS3TRC_OUT_MAG_RAW_Z_H        = 0x6B;
    const static uint8_t LSM6DS3TRC_X_OFS_USR              = 0x73;
    const static uint8_t LSM6DS3TRC_Y_OFS_USR              = 0x74;
    const static uint8_t LSM6DS3TRC_Z_OFS_USR              = 0x75;

    // Linearaccelerationoutdatarate
    const static uint8_t LSM6DS3TRC_ACC_RATE_0      = 0x00;
    const static uint8_t LSM6DS3TRC_ACC_RATE_1HZ6   = 0xB0;
    const static uint8_t LSM6DS3TRC_ACC_RATE_12HZ5  = 0x10;
    const static uint8_t LSM6DS3TRC_ACC_RATE_26HZ   = 0x20;
    const static uint8_t LSM6DS3TRC_ACC_RATE_52HZ   = 0x30;
    const static uint8_t LSM6DS3TRC_ACC_RATE_104HZ  = 0x40;
    const static uint8_t LSM6DS3TRC_ACC_RATE_208HZ  = 0x50;
    const static uint8_t LSM6DS3TRC_ACC_RATE_416HZ  = 0x60;
    const static uint8_t LSM6DS3TRC_ACC_RATE_833HZ  = 0x70;
    const static uint8_t LSM6DS3TRC_ACC_RATE_1660HZ = 0x80;
    const static uint8_t LSM6DS3TRC_ACC_RATE_3330HZ = 0x90;
    const static uint8_t LSM6DS3TRC_ACC_RATE_6660HZ = 0xA0;

    // Lineargyroscopeoutdatarate
    const static uint8_t LSM6DS3TRC_GYR_RATE_0      = 0x00;
    const static uint8_t LSM6DS3TRC_GYR_RATE_1HZ6   = 0xB0;
    const static uint8_t LSM6DS3TRC_GYR_RATE_12HZ5  = 0x10;
    const static uint8_t LSM6DS3TRC_GYR_RATE_26HZ   = 0x20;
    const static uint8_t LSM6DS3TRC_GYR_RATE_52HZ   = 0x30;
    const static uint8_t LSM6DS3TRC_GYR_RATE_104HZ  = 0x40;
    const static uint8_t LSM6DS3TRC_GYR_RATE_208HZ  = 0x50;
    const static uint8_t LSM6DS3TRC_GYR_RATE_416HZ  = 0x60;
    const static uint8_t LSM6DS3TRC_GYR_RATE_833HZ  = 0x70;
    const static uint8_t LSM6DS3TRC_GYR_RATE_1660HZ = 0x80;
    const static uint8_t LSM6DS3TRC_GYR_RATE_3330HZ = 0x90;
    const static uint8_t LSM6DS3TRC_GYR_RATE_6660HZ = 0xA0;

    // Accelerometerfull-scale
    const static uint8_t LSM6DS3TRC_ACC_FSXL_2G  = 0x00;
    const static uint8_t LSM6DS3TRC_ACC_FSXL_16G = 0x04;
    const static uint8_t LSM6DS3TRC_ACC_FSXL_4G  = 0x08;
    const static uint8_t LSM6DS3TRC_ACC_FSXL_8G  = 0x0C;

    // Gyroscopefull-scale
    const static uint8_t LSM6DS3TRC_GYR_FSG_245  = 0x00;
    const static uint8_t LSM6DS3TRC_GYR_FSG_500  = 0x04;
    const static uint8_t LSM6DS3TRC_GYR_FSG_1000 = 0x08;
    const static uint8_t LSM6DS3TRC_GYR_FSG_2000 = 0x0C;

    // Accelerometeranalogchainbandwidth
    const static uint8_t LSM6DS3TRC_ACC_BW0XL_1500HZ = 0x00;
    const static uint8_t LSM6DS3TRC_ACC_BW0XL_400HZ  = 0x01;

    // Accelerometerbandwidthselection
    const static uint8_t LSM6DS3TRC_ACC_LOW_PASS_ODR_50  = 0x88;
    const static uint8_t LSM6DS3TRC_ACC_LOW_PASS_ODR_100 = 0xA8;
    const static uint8_t LSM6DS3TRC_ACC_LOW_PASS_ODR_9   = 0xC8;
    const static uint8_t LSM6DS3TRC_ACC_LOW_PASS_ODR_400 = 0xE8;

    const static uint8_t LSM6DS3TRC_ACC_HIGH_PASS_ODR_50  = 0x04;
    const static uint8_t LSM6DS3TRC_ACC_HIGH_PASS_ODR_100 = 0x24;
    const static uint8_t LSM6DS3TRC_ACC_HIGH_PASS_ODR_9   = 0x44;
    const static uint8_t LSM6DS3TRC_ACC_HIGH_PASS_ODR_400 = 0x64;

    // CTRL4_Cregister
    const static uint8_t LSM6DS3TRC_CTRL4_DEN_XL_EN_DISABLE     = 0x00;
    const static uint8_t LSM6DS3TRC_CTRL4_DEN_XL_EN_ENABLE      = 0x80;
    const static uint8_t LSM6DS3TRC_CTRL4_SLEEP_ENABLE          = 0x40;
    const static uint8_t LSM6DS3TRC_CTRL4_SLEEP_DISABLE         = 0x00;
    const static uint8_t LSM6DS3TRC_CTRL4_DEN_DRDY_INT1_DISBALE = 0x00;
    const static uint8_t LSM6DS3TRC_CTRL4_DEN_DRDY_INT1_ENABLE  = 0x20;
    const static uint8_t LSM6DS3TRC_CTRL4_DRDY_MASK_DISABLE     = 0x00;
    const static uint8_t LSM6DS3TRC_CTRL4_DRDY_MASK_ENABLE      = 0x08;
    const static uint8_t LSM6DS3TRC_CTRL4_I2C_DISABLE           = 0x04;
    const static uint8_t LSM6DS3TRC_CTRL4_I2C_ENABLE            = 0x00;
    const static uint8_t LSM6DS3TRC_CTRL4_LPF1_SELG_ENABLE      = 0x02;
    const static uint8_t LSM6DS3TRC_CTRL4_LPF1_SELG_DISABLE     = 0x00;

    // CTRL6_Cregister
    const static uint8_t LSM6DS3TRC_CTRL6_C_EDGE_TRIGGER       = 0x80;
    const static uint8_t LSM6DS3TRC_CTRL6_C_LEVEL_TRIGGER      = 0x40;
    const static uint8_t LSM6DS3TRC_CTRL6_C_LEVEL_LATCHED      = 0x60;
    const static uint8_t LSM6DS3TRC_CTRL6_C_LEVEL_FIFO         = 0xC0;
    const static uint8_t LSM6DS3TRC_CTRL6_C_XL_HM_MODE_ENABLE  = 0x00;
    const static uint8_t LSM6DS3TRC_CTRL6_C_XL_HM_MODE_DISABLE = 0x10;
    const static uint8_t LSM6DS3TRC_CTRL6_C_FTYPE_1            = 0x00;
    const static uint8_t LSM6DS3TRC_CTRL6_C_FTYPE_2            = 0x01;
    const static uint8_t LSM6DS3TRC_CTRL6_C_FTYPE_3            = 0x02;
    const static uint8_t LSM6DS3TRC_CTRL6_C_FTYPE_4            = 0x03;

    // CTRL7_Gregister
    const static uint8_t LSM6DS3TRC_CTRL7_G_HM_MODE_ENABLE          = 0x00;
    const static uint8_t LSM6DS3TRC_CTRL7_G_HM_MODE_DISABLE         = 0x80;
    const static uint8_t LSM6DS3TRC_CTRL7_G_HP_EN_DISABLE           = 0x00;
    const static uint8_t LSM6DS3TRC_CTRL7_G_HP_EN_ENABLE            = 0x40;
    const static uint8_t LSM6DS3TRC_CTRL7_G_HPM_16MHZ               = 0x00;
    const static uint8_t LSM6DS3TRC_CTRL7_G_HPM_65MHZ               = 0x10;
    const static uint8_t LSM6DS3TRC_CTRL7_G_HPM_260MHZ              = 0x20;
    const static uint8_t LSM6DS3TRC_CTRL7_G_HPM_1HZ04               = 0x30;
    const static uint8_t LSM6DS3TRC_CTRL7_G_ROUNDING_STATUS_DISABLE = 0x04;
    const static uint8_t LSM6DS3TRC_CTRL7_G_ROUNDING_STATUS_ENABLE  = 0x00;

    const static uint8_t LSM6DS3TRC_STATUS_TEMPERATURE   = 0x04;
    const static uint8_t LSM6DS3TRC_STATUS_GYROSCOPE     = 0x02;
    const static uint8_t LSM6DS3TRC_STATUS_ACCELEROMETER = 0x01;

    // const static uint8_t LSM6DS3TRC_MODE = 0;
    // const static uint8_t LSM6DS3TRC_CSPIN = 0;
    const static uint8_t LSM6DS3TRC_WHO_AM_I_RESP = 0x6A;
};

typedef struct
{
    char  dev[64];
    int   ypr[3];
    int   gyro[3];
    int   accel[3];
    float compass[3];

    int16_t a[3]; // [x, y, z]            accel vector
    int16_t g[3]; // [x, y, z]            gyro vector
    int32_t _q[4];
    int32_t t;
    int16_t c[3];
    uint8_t sample_rate;
} YQMpuPrivData;

const static uint8_t            LSM6DS3TRC_ADDRESS = 0x6A;
std::shared_ptr<Mpu_Lsm6ds3trc> mpu_lsm6ds3trc;
static int                      yq_mpu_impl_lsm6ds3trc_open(YQMpuContext *ctx, const char *dev)
{
    if (!ctx)
    {
        return YQ_ERROR_ARGS;
    }
    ctx->priv_data = NULL;

    YQMpuPrivData *_priv = (YQMpuPrivData *)malloc(sizeof(YQMpuPrivData));
    if (!_priv)
    {
        return YQ_ERROR_NO_MEM;
    }

    mpu_lsm6ds3trc = std::make_shared<Mpu_Lsm6ds3trc>(dev, LSM6DS3TRC_ADDRESS);

    _priv->sample_rate = 200; // 最大采样周期200hz
    strncpy(_priv->dev, dev, sizeof(_priv->dev) - 1);
    ctx->priv_data = _priv;

    bool ret = mpu_lsm6ds3trc->Lsm6ds3trc_Init();
    if (!ret)
    {
        LOGD("mpu init fail [%d]", ret);
        free(_priv);
        ctx->priv_data = NULL;
        return -2;
    }

    return YQ_ERROR_SUCCESS;
}

static int yq_mpu_impl_lsm6ds3trc_close(YQMpuContext *ctx)
{
    if (!ctx || !ctx->priv_data)
    {
        return YQ_ERROR_ARGS;
    }

    YQMpuPrivData *_priv = (YQMpuPrivData *)ctx->priv_data;

    free(_priv);
    ctx->priv_data = NULL;
    mpu_lsm6ds3trc = NULL;

    return YQ_ERROR_SUCCESS;
}

static int yq_mpu_impl_lsm6ds3trc_ready(YQMpuContext *ctx)
{
    if (!ctx || !ctx->priv_data)
    {
        return -1;
    }

    if (!mpu_lsm6ds3trc->Lsm6ds3trc_Ready())
    {
        return -2;
    }

    return 0;
}

static int yq_mpu_impl_lsm6ds3trc_read_ypr(YQMpuContext *ctx, float ypr[])
{
    if (!ctx || !ctx->priv_data)
    {
        return YQ_ERROR_ARGS;
    }

    YQMpuPrivData *_priv      = (YQMpuPrivData *)ctx->priv_data;
    bool           read_error = mpu_lsm6ds3trc->Lsm6ds3trc_Get_YawPitchRoll(ypr);
    if (read_error)
    {
        return YQ_ERROR_FILE_READ;
    }

    return YQ_ERROR_DONE;
}

static int yq_mpu_impl_lsm6ds3trc_read_gyro(YQMpuContext *ctx, float gyro[], float accel[])
{
    if (!ctx || !ctx->priv_data)
    {
        return YQ_ERROR_ARGS;
    }

    YQMpuPrivData *_priv      = (YQMpuPrivData *)ctx->priv_data;
    bool           read_error = mpu_lsm6ds3trc->Lsm6ds3trc_Get_gyro(gyro);
    if (read_error)
    {
        return YQ_ERROR_FILE_READ;
    }

    return YQ_ERROR_DONE;
}

#define MPU_IMPL_NAME "lsm6ds3trc"
#define MPU_DEVICE_NAME "/dev/i2c-1"

static char impl_str[] = MPU_IMPL_NAME;
static char device_str[] = MPU_DEVICE_NAME;

const YQMpuImpl yq_mpu_impl_lsm6ds3trc = {
    .impl = impl_str,
    .device = device_str,
    .open = yq_mpu_impl_lsm6ds3trc_open,
    .close = yq_mpu_impl_lsm6ds3trc_close,
    .ready = yq_mpu_impl_lsm6ds3trc_ready,
    .read_ypr = yq_mpu_impl_lsm6ds3trc_read_ypr,
    .read_gyro = yq_mpu_impl_lsm6ds3trc_read_gyro,
};