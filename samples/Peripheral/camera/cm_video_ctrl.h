/**
 * @file cm_video_ctrl.h
 * @author lijunhong
 */
#ifndef CM_VIDEO_CTRL_H
#define CM_VIDEO_CTRL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "cm_video_interface.h"

int cm_video_take_photo_save_to_file(const char *file_path);
int cm_video_impl_init(const char *type);
void cm_video_impl_deinit(void);
#ifdef __cplusplus
}
#endif

#endif // CM_VIDEO_CTRL_H
