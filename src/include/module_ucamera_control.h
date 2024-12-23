/*********************************************************
 * File Name   : module_ucamera_control.h
 * Author      : Tania Xiang
 * Mail        : xiuhui.xiang@ingenic.com
 * Created Time: 2022-05-31 10:54
 ********************************************************/

#ifndef __MODULE_UVC_CONTROL_H__
#define __MODULE_UVC_CONTROL_H__

#include <global_config.h>
#include <usbcamera.h>

int module_ucamera_stream_on(void);
int module_ucamera_impinited(void);
void module_ucamera_get_frame(uint32_t *frame_width, uint32_t *frame_height);
int module_ucamera_init(void *param);
void module_ucamera_deinit();
void *module_ucamera_getCtHandler(void);
void *module_ucamera_getPuHandler(void);
void module_ucamera_setCtHandler(void *ct);
void module_ucamera_setPuHandler(void *pu);
#endif

