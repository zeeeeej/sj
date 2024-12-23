/*************************************************************************
* Copyright (C) 2005~2022 Ingenic Semiconductor Co.,Ltd
*
* File Name    : uvc_xu_control.h
* Author       : wxyang
* Mail         : wxyang@ingenic.com
* Created Time : 2022-07-30 09:32
*************************************************************************/

#ifndef __UVC_XU_CONTROL_H
#define __UVC_XU_CONTROL_H

int sample_video2_eu_set(int cmd, void *data, int len);
int sample_video2_eu_get(int cmd, int req, void *data);

#endif
