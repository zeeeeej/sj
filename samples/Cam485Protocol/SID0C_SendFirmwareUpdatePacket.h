#ifndef SID0C_SENDFIRMWAREUPDATEPACKET
#define SID0C_SENDFIRMWAREUPDATEPACKET
#include <stdio.h>
#include <stdint.h>
#include "SID0BC_FirmwareUpdateInfo.h"

#define SID0C_MSG_RESP_DATA_LEN      (1)
#define SID0C_MSG_RESP_TOTAL_LEN     (11)

int SID0C_SendFirmwareUpdatePacket(uint8_t *msg_buf, uint32_t msg_dlc);

#define OTASHELLDATA (" \
#!/bin/bash\n" \
"cd  " FILE_DIR "\n" \
"rm sample_camera_rst\n \
xz_file=\"sample_camera_rst.xz\"\n \
unxz_file=\"sample_camera_rst\"\n \
md5_output_file=\"/system/md5.ini\"\n \
df -h\n \
ls /system/bin -l\n \
/system/bin/xz -d \"$xz_file\"\n \
if [ $? -eq 0 ]; then\n \
    echo \"xz $xz_file ok\"\n \
    md5_value=$(md5sum \"$unxz_file\" | awk '{print $1}')\n \
    echo \"$md5_value\" > \"$md5_output_file\"\n \
else\n \
    echo \"xz $xz_file error\"\n \
    df -h\n \
    ls /system/bin -l\n \
    rm sample_camera_rst\n \
    rm sample_camera_rst.xz \n \
    df -h\n \
    ls /system/bin -l\n \
    reboot \n \
    exit \n \
fi\n \
chmod 777  sample_camera_rst\n \
df -h\n \
ls /system/bin -l\n \
echo \"cp sample_camera_rst  /system/bin -f\"\n \
cp  sample_camera_rst  /system/bin/ -f\n \
df -h\n \
ls /system/bin -l\n \
echo sync\n \
sync\n \
echo sync complete\n \
echo reboot \n \
reboot\n \
cd -\n \
")

#endif