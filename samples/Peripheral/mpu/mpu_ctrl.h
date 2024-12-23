#ifndef __MPU_CTRL_H
#define __MPU_CTRL_H


int gyroscope_close(void);
int gyroscope_ready(void);
int gyroscope_read_yqr(float ypr[3]);
int gyroscope_read_gyro(float gyro[3], float accel[3]);
int gyroscope_open(void);

#endif
