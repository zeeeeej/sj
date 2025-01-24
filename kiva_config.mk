#########################################################################
# File Name: kiva_config.mk
# Author: Tania Xiang
# mail: xiuhui.xiang@ingenic.com
# Created Time: 2022-11-28 16:17
#########################################################################

# uvc config
# uvc protocol: UVC_1_0 / UVC_1_1 / UVC_1_5
KIVA_UVC_TYPE=UVC_1_1

# sensor num : 1(T31 T40) / 2 (T40)
KIVA_SENSOR_NUM=1

# uvc video num : 1 / 2
KIVA_VIDEO_NUM=1

# uvc winhello
KIVA_WINHELLO=n

# uvc kiva quick start
KIVA_QUICK_START=n

#audio_ns: AI NS
KIVA_MERT=n

#audio mmse + ns : 4dmic
KIVA_MMSE=n

#IVDC_MODE: T41(y/n)
KIVA_IVDC=n

# uvc modules
# ucamera: uvc+uac
KIVA_MODULE_UCAMERA=y
KIVA_MODULE_UVC=y
KIVA_MODULE_UAC=y

# config file
KIVA_MODULE_CONFIG=y

# led control
KIVA_MODULE_LED_CONTROL=n

# auto focus
KIVA_MODULE_AUTOFOCUS=n

# face auto focus
KIVA_MODULE_FASTAF=n

#face zoom: auto centering or auto framing
KIVA_MODULE_FACEZOOM=n

#pip
KIVA_MODULE_PIP=n

#ptz:
KIVA_MODULE_PTZ=n

#osd
KIVA_ISP_OSD=n
KIVA_IPU_OSD=n

# remote
KIVA_MODULE_REMOTE=n

# optzoom
KIVA_MODULE_OPTZOOM=n

KIVA_MODULE_FACEAE=n

#high resolution
KIVA_HIGH_RESOLUTION=n

#gesture recognition algorithm
KIVA_MODULE_HAND=n
