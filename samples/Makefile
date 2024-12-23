include ../kiva_config.mk
include $(KIVA_ENV_MODULES_DIR)/kiva_modules.mk

###########
#toolchains
############
CROSS_COMPILE := $(KIVA_ENV_GCC_PATH)/mips-linux-gnu-
COMPILE_TYPE := $(KIVA_ENV_LIBC_TYPE)

UVC_TYPE := $(KIVA_UVC_TYPE)
VIDEO_NUM := $(KIVA_VIDEO_NUM)

CC = $(CROSS_COMPILE)gcc
CPP = $(CROSS_COMPILE)g++
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
OBJS := $(KIVA_ENV_SRC_DIR)/common/$(IMP_COMMON)/imp_common.o \
	$(KIVA_ENV_SRC_DIR)/common/$(IMP_COMMON)/imp_video_ctl.o \
	$(KIVA_ENV_SRC_DIR)/common/$(IMP_COMMON)/imp_audio_ctl.o \
	$(KIVA_ENV_SRC_DIR)/common/load_config.o \

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
		-I$(KIVA_ENV_SRC_DIR)/

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

# include adapt some ubuntu system
CFLAGS += $(INCLUDES) -O2 -Wall -march=mips32r2 -std=gnu99 -Werror

export CFLAGS LIBS INCLUDES ARFLAG
export CC CPP AR LD



##############################
#### sample target ###########
##############################

TARGET := sample_ucamera

all: $(TARGET)

#### samples #########
sample_ucamera: sample_ucamera.o $(OBJS) modules utils
	$(CC) $(CFLAGS) sample_ucamera.o $(OBJS) $(LDFLAG) -o $@ $(SDK_LIBS) $(LIBS) $(UTILS_LIBS)
	$(STRIP) $@

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

mpu_test:
	$(CC) $(CFLAGS) -o mpu-test LSM6DS3TRC_Test.c -lm

%o:%c
	$(CC) $(CFLAGS) $(LIBS) -o $@ -c $^

clean:	modules_clean \
	utils_clean
	rm -rf $(TARGET) $(OBJS)
	rm -rf *.o *~

distclean: modules_clean  utils_clean
	rm -rf $(TARGET) $(OBJS)
