

include ../kiva_config.mk
include $(KIVA_ENV_MODULES_DIR)/kiva_modules.mk

###########
#toolchains
############
CROSS_COMPILE := $(KIVA_ENV_GCC_PATH)/mips-linux-gnu-
COMPILE_TYPE := $(KIVA_ENV_LIBC_TYPE)

UVC_TYPE := $(KIVA_UVC_TYPE)
VIDEO_NUM := $(KIVA_VIDEO_NUM)

FLOAT_FLAGS =  -mhard-float
FLOAT_FLAGS += -ffast-math

CC = $(CROSS_COMPILE)gcc
CPP = $(CROSS_COMPILE)g++
$(info CROSS_COMPILE=$(CROSS_COMPILE))
$(info CPP=$(CPP))
LD = $(CROSS_COMPILE)ld
AR = $(CROSS_COMPILE)ar
STRIP = $(CROSS_COMPILE)strip

ARFLAG := -rcs

#######################
#kiva version
#######################
CFLAGS += -DKIVA_VERSION=\"$(KIVA_ENV_VERSION)\"

#######################
#######################
# libc type
ifeq ($(KIVA_ENV_LIBC_TYPE), uclibc)
	CFLAGS += -muclibc
endif

# winhello
ifeq ($(KIVA_WINHELLO), y)
	CFLAGS += -DWINHELLO_ENABLE
endif

# uvc type
ifeq ($(KIVA_UVC_TYPE), UVC_1_0)
	CFLAGS += -DCONFIG_UVC_1_0
else ifeq ($(KIVA_UVC_TYPE), UVC_1_1)
	CFLAGS += -DCONFIG_UVC_1_1
else
	CFLAGS += -DCONFIG_UVC_1_5
endif

# sdk
KIVA_SDK_DIR = $(KIVA_ENV_LIBS_DIR)/t23_sdk/
IMP_COMMON := imp
HOST := T23

CFLAGS += -D$(HOST) -D$(KIVA_ENV_HOST)

# video num
ifeq ($(KIVA_VIDEO_NUM), 2)
	CFLAGS += -DDUAL_VIDEO
endif

CFLAGS += -DDUAL_SENSOR=0

#### uvc and uac common source #######

CFLAGS += -I. \
	-I$(KIVA_ENV_LIBS_DIR)/modbus/include

OBJS := $(KIVA_ENV_SRC_DIR)/common/$(IMP_COMMON)/imp_common.o \
    $(KIVA_ENV_SRC_DIR)/common/$(IMP_COMMON)/imp_video_ctl.o \
    $(KIVA_ENV_SRC_DIR)/common/$(IMP_COMMON)/imp_audio_ctl.o \
    $(KIVA_ENV_SRC_DIR)/common/load_config.o \

OBJS += $(KIVA_ENV_SRC_DIR)/../samples/Peripheral/uart/cm_uart.o \
	$(KIVA_ENV_SRC_DIR)/../samples/Peripheral/camera/cm_video_ctrl.o \
	$(KIVA_ENV_SRC_DIR)/../samples/Peripheral/camera/cm_video_ctrl_t23.o \
	$(KIVA_ENV_SRC_DIR)/../samples/Peripheral/camera/sample-common.o \
	$(KIVA_ENV_SRC_DIR)/../samples/Peripheral/mpu/yq_mpu_impl_lsm6ds3trc.o \
	$(KIVA_ENV_SRC_DIR)/../samples/Peripheral/mpu/mpu_ctrl.o \
	$(KIVA_ENV_SRC_DIR)/../samples/Peripheral/wdt/wdt.o \
    $(KIVA_ENV_SRC_DIR)/../samples/door_detect/door_detect.o \
	$(KIVA_ENV_SRC_DIR)/../samples/modbus_task/wind_connect.o \
	$(KIVA_ENV_SRC_DIR)/../samples/modbus_task/wind_process_manager.o \
	$(KIVA_ENV_SRC_DIR)/../samples/modbus_task/wind_protocol_wrap.o \
	$(KIVA_ENV_SRC_DIR)/../samples/modbus_task/wind_modbus_wrap.o \
	$(KIVA_ENV_SRC_DIR)/../samples/modbus_task/wind_connect_up.o \
	$(KIVA_ENV_SRC_DIR)/../samples/modbus_task/wind_protocol_wrap_up.o \
	$(KIVA_ENV_SRC_DIR)/../samples/modbus_task/md5.o \
	$(KIVA_ENV_SRC_DIR)/../samples/utils/cm_base64.o \
	$(KIVA_ENV_SRC_DIR)/../samples/utils/cm_conf.o \
	$(KIVA_ENV_SRC_DIR)/../samples/utils/cm_list.o \
	$(KIVA_ENV_SRC_DIR)/../samples/utils/cm_utils.o \
	$(KIVA_ENV_SRC_DIR)/../samples/utils/cJSON.o \
	$(KIVA_ENV_SRC_DIR)/../samples/utils/jkbytes.o \
    $(KIVA_ENV_SRC_DIR)/../samples/cm_config.o \
	$(KIVA_ENV_SRC_DIR)/../samples/self_check/cam_self_check.o \
	$(KIVA_ENV_SRC_DIR)/../samples/self_check/self_check.o \
	$(KIVA_ENV_SRC_DIR)/../samples/self_check/gyro_self_check.o \
	$(KIVA_ENV_SRC_DIR)/../samples/tmp/ota_process.o \
	$(KIVA_ENV_SRC_DIR)/../samples/tmp/take_photo.o \
	$(KIVA_ENV_SRC_DIR)/../samples/tmp/MessageDispatcher.o \
	$(KIVA_ENV_SRC_DIR)/../samples/tmp/MessageDispaterPort.o \
	$(KIVA_ENV_SRC_DIR)/../samples/tmp/trans_door_image.o \
	$(KIVA_ENV_SRC_DIR)/../samples/Peripheral/usb/wind_usb_connect.o \
	$(KIVA_ENV_SRC_DIR)/../samples/log/circular_log.o \
	$(KIVA_ENV_SRC_DIR)/../samples/command/cli_command.o \
	$(KIVA_ENV_SRC_DIR)/../samples/log/debug_logger.o \
	$(KIVA_ENV_SRC_DIR)/../samples/tmp/file_trans.o \
	$(KIVA_ENV_SRC_DIR)/../samples/Cam485Protocol/Heartbeat.o

	
ifeq ($(KIVA_ISP_OSD), y)
	CFLAGS += -DOSD_ENABLE
	OBJS += $(KIVA_ENV_SRC_DIR)/common/$(IMP_COMMON)/imp_isposd.o
endif

ifeq ($(KIVA_MODULE_PIP), y)
	KIVA_IPU_OSD=y
endif

ifeq ($(KIVA_IPU_OSD), y)
	CFLAGS += -DIPU_OSD_ENABLE
	OBJS += $(KIVA_ENV_SRC_DIR)/common/$(IMP_COMMON)/imp_ipu_osd.o
endif

ifeq ($(FACEDET_ENABLE), y)
	OBJS += $(KIVA_ENV_SRC_DIR)/common/$(IMP_COMMON)/imp_facedet.o
endif

#### include path ##############
INCLUDES +=	-I$(KIVA_ENV_LIBS_DIR)/ucamcore/include \
		-I$(KIVA_SDK_DIR)/include \
		-I$(KIVA_ENV_UTILS_DIR)/include \
		-I$(KIVA_ENV_SRC_DIR)/include \
		-I$(KIVA_ENV_SRC_DIR)/common/ \
		-I$(KIVA_ENV_SRC_DIR)/common/$(IMP_COMMON)/ \
		-I$(KIVA_ENV_SRC_DIR)/ \
		-I$(KIVA_ENV_TOP)/samples/self_check/ \
		-I$(KIVA_ENV_TOP)/samples/utils/ \
		-I$(KIVA_ENV_TOP)/samples/modbus_task/ \
		-I$(KIVA_ENV_TOP)/samples/Peripheral/uart/ \
		-I$(KIVA_ENV_TOP)/samples/Peripheral/camera/ \
		-I$(KIVA_ENV_TOP)/samples/Peripheral/mpu/ \
		-I$(KIVA_ENV_TOP)/samples/Peripheral/wdt/ \
		-I$(KIVA_ENV_TOP)/samples/door_detect \
		-I$(KIVA_ENV_TOP)/samples/tmp \
		-I$(KIVA_ENV_TOP)/samples/Peripheral/usb \
		-I$(KIVA_ENV_TOP)/samples/log \
		-I$(KIVA_ENV_TOP)/samples/command \
		-I$(KIVA_ENV_TOP)/samples/Cam485Protocol 

ifeq ($(KIVA_ISP_OSD), y)
	INCLUDES += -I$(KIVA_ENV_LIBS_DIR)/png/include/$(HOST)
endif

#### imp sdk_lib ############
SDK_LIBS += 	-L$(KIVA_SDK_DIR)/lib/$(COMPILE_TYPE) -limp \
		-L$(KIVA_SDK_DIR)/lib/$(COMPILE_TYPE) -lalog \
		-L$(KIVA_SDK_DIR)/lib/$(COMPILE_TYPE) -lsysutils \

#### modules lib ############
#### ucamcore lib ###########
LIBS +=		-L$(KIVA_ENV_MODULES_DIR) -lmodules \
		-L$(KIVA_ENV_LIBS_DIR)/ucamcore/lib/$(COMPILE_TYPE) -lusbcamera \

LIBS += -L$(KIVA_ENV_LIBS_DIR)/modbus/lib -lmodbus

#### utils lib ###############
UTILS_LIBS += -L$(KIVA_ENV_UTILS_DIR)/ -lutils

#### isposd lib ##############
ifeq ($(KIVA_ISP_OSD), y)
	LIBS += -L$(KIVA_ENV_LIBS_DIR)/png/lib/$(HOST)/$(KIVA_ENV_LIBC_TYPE) -lpng16 -lz
endif

LIBS += -Wl,-Bdynamic -lpthread -lm -ldl -lrt -lstdc++ -std=gnu99

.PHONY: modules modules_clean \
	utils utils_clean \
	clean
.PHONY: ota_test
.PHONY: mpu_test
# include adapt some ubuntu system
CFLAGS += $(INCLUDES) -O2 -Wall -march=mips32r2 -std=gnu99 $(FLOAT_FLAGS)

export CFLAGS LIBS INCLUDES ARFLAG
export CC CPP AR LD

CXXFLAGS := $(filter-out -std=gnu99,$(CFLAGS))
CXXFLAGS += -std=c++11 -DMODULE_UVC -DMODULE_UAC -DKIVA_VERSION=\"$(KIVA_ENV_VERSION)\" \
            -muclibc -DCONFIG_UVC_1_1 -DT23 -DT23N -DDUAL_SENSOR=0 \
            -O2 -Wall -march=mips32r2 -Werror -Wno-unused-but-set-variable -Wno-unused-variable $(FLOAT_FLAGS)

##############################
#### sample target ###########
##############################

TARGET := sample_camera_rst ota_test mpu_test

all: $(TARGET)

#### samples #########
sample_camera_rst: sample_camera_rst.o $(OBJS) modules utils
	$(CPP) $(CFLAGS) $(CXXFLAGS) sample_camera_rst.o $(OBJS) $(LDFLAG) -o $@ $(SDK_LIBS) $(UTILS_LIBS) $(LIBS)
	$(STRIP) $@
# 添加 ota_test 的编译规则


# 修改 ota_test 的编译规则




#### modules and utils ###########
modules:
	make -C $(KIVA_ENV_MODULES_DIR)

modules_clean:
	make -C $(KIVA_ENV_MODULES_DIR) clean

utils:
	make -C $(KIVA_ENV_UTILS_DIR)

utils_clean:
	make -C $(KIVA_ENV_UTILS_DIR) clean

cm_uart:
	$(CC) $(CFLAGS) -o cm_uart_test cm_uart_test.c cm_uart.c



%.o: %.cpp
	$(CPP) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

%o:%c
	$(CC) $(CFLAGS) $(LIBS) -o $@ -c $^

clean:	modules_clean \
	utils_clean
	rm -rf $(TARGET) $(OBJS)
	rm -rf *.o *~

distclean: modules_clean  utils_clean
	rm -rf $(TARGET) $(OBJS)


