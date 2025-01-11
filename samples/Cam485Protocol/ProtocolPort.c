#include "ProtocolPort.h"
#include "cm_video_ctrl.h"
#include "stdint.h"
uint8_t request_take_photo(char *file)
{
    return cm_video_take_photo_save_to_file(file);
}