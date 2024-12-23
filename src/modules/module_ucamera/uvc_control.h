#ifndef __MODULE_UVC_CONTROL_H__
#define __MODULE_UVC_CONTROL_H__

#include <global_config.h>
#include <usbcamera.h>

#define MODULE_UVC_CONTROL_NAME	"uvc_control"
/**
 * init and deinit.
 */

void uvc_control_enable_impinited(int video_index);
int uvc_control_get_stream_status(int video_index);
void uvc_control_get_frame(uint32_t *frame_width,uint32_t *frame_height);
int uvc_control_init(void *param);
void uvc_control_deinit();

int sample_video1_eu_set(int cmd, void *data, int len);
int sample_video1_eu_get(int cmd, int req, void *data);

int sample_video2_eu_set(int cmd, void *data, int len);
int sample_video2_eu_get(int cmd, int req, void *data);
#endif /* __MODULE_UVC_CONTROL_H__ */
