/*
 * imp_isp.h
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 */

#include <stdio.h>
#include <stdlib.h>
#include <imp/imp_isp.h>
#include <imp/imp_log.h>

int sample_get_ev(void);

int sample_wait_frame_down(int timeout);

int sample_get_sensor_attr(IMPISPSENSORAttr *attr);

