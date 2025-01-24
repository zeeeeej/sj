#ifndef __MODULE_UAC_CONTROL_H__
#define __MODULE_UAC_CONTROL_H__

#include <global_config.h>
#include <usbcamera.h>

#define MODULE_UAC_CONTROL_NAME	"uac_control"

/**
 * init and deinit.
 */
/* int ModuleUAC_Init(struct Ucamera_Cfg *ucfg); */
void ModuleUAC_Deinit(void);

void ModuleUAC_Set_Config(struct Ucamera_Cfg *ucfg, uvc_base_info_t uvc_base_info);
void ModuleUAC_SetFunc();
void ModuleUAC_AudioStart(void);

#endif /* __MODULE_UAC_CONTROL_H__ */
