#ifndef __MODULE_UVC_CONTROL_H__
#define __MODULE_UVC_CONTROL_H__

#include <global_config.h>
#include <usbcamera.h>

#define MODULE_UVC_CONTROL_NAME	"uvc_control"
//#########pantilt
typedef struct _crop_obj {
	int left;
	int top;
	int w;
	int h;
} crop_obj_t;

/**
 * init and deinit.
 */
/* int ModuleUVC_Init(struct Ucamera_Cfg *ucfg, uvc_base_info_t uvc_base_info); */
void ModuleUVC_Deinit(void);

void ModuleUVC_Enable_ImpInited();

int ModuleUVC_Set_Config(struct Ucamera_Cfg *ucfg, uvc_base_info_t uvc_base_info);
void ModuleUVC_SetFunc(void);
void ModuleUVC_SetPU(void);
void ModuleUVC_SetCT(void);
void ModuleUVC_SetEU(void);
void ModuleUVC_RegisterEvent(void);
void ModuleUVC_VideoStart(void);

#endif /* __MODULE_UVC_CONTROL_H__ */
