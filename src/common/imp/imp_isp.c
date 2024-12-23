/*
 * imp_isp.c
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 */

#include "imp_isp.h"

int sample_get_ev()
{
	int ret = -1;

	IMPISPEVAttr attr;
	ret = IMP_ISP_Tuning_GetEVAttr(&attr);
	if(ret){
		printf("IMP_ISP_Tuning_GetEVAttr error !\n");
		return -1;
	}
	return attr.ev + 1;
}

int sample_wait_frame_down(int timeout)
{
	int ret = -1;
	IMPISPWaitFrameAttr attr;

	attr.timeout = timeout;
	ret = IMP_ISP_Tuning_WaitFrame(&attr);
	if(ret){
		printf("IMP_ISP_Tuning_WaitFrame error!\n");
		return -1;
	}
	return 0;
}

int sample_get_sensor_attr(IMPISPSENSORAttr *attr)
{
	int ret = -1;

	ret = IMP_ISP_Tuning_GetSensorAttr(attr);
	if(ret){
		printf("IMP_ISP_Tuning_GetSensorAttr error !\n");
		return -1;
	}
	return 0;
}


