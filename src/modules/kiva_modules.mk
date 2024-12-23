
include $(KIVA_ENV_TOP)/kiva_config.mk

# module ucamera
ifeq ($(KIVA_MODULE_UCAMERA), y)

ifeq ($(KIVA_WINHELLO), y)
	MODULES += module_uvc_xu
	MODULES_CLEAN += module_uvc_xu_clean

	MODULES_LIBS += libuvc_xu.a
endif

ifeq ($(KIVA_MODULE_UVC), y)
	CFLAGS += -DMODULE_UVC
endif
ifeq ($(KIVA_MODULE_UAC), y)
	CFLAGS += -DMODULE_UAC
endif

	MODULES += module_ucamera
	MODULES_CLEAN += module_ucamera_clean

	MODULES_LIBS += libucamera.a
endif

# module config
ifeq ($(KIVA_MODULE_CONFIG), y)
	MODULES += module_config
	MODULES_CLEAN += module_config_clean

	MODULES_LIBS += libconfig.a
endif

# module led control
ifeq ($(KIVA_MODULE_LED_CONTROL), y)
	MODULES += module_led
	MODULES_CLEAN += module_led_clean

	MODULES_LIBS += libled.a
endif

# module auto focus
ifeq ($(KIVA_MODULE_AUTOFOCUS), y)
	CFLAGS += -DMODULE_AUTOFOCUS_ENABLE

	MODULES += module_autofocus
	MODULES_CLEAN += module_autofocus_clean

	MODULES_LIBS += libaf.a
endif

ifeq ($(KIVA_MODULE_FACEAF), y)
	CFLAGS += -DMODULE_FACEAF_ENABLE

	FACEDET_ENABLE=y
	MODULES += module_autofocus
	MODULES_CLEAN += module_autofocus_clean

	MODULES_LIBS += libaf.a
endif
# autoframing or autocentering
ifeq ($(KIVA_MODULE_FACEZOOM), y)
	CFLAGS += -DMODULE_FACEZOOM_ENABLE
	FACEDET_ENABLE=y

	MODULES += module_facezoom
	MODULES_CLEAN += module_facezoom_clean

	MODULES_LIBS += libfacezoom.a
endif

# pip: picture in picture
ifeq ($(KIVA_MODULE_PIP), y)
	KIVA_IPU_OSD=y
	CFLAGS += -DMODULE_PIP_ENABLE
	FACEDET_ENABLE=y

MODULES += module_pip
MODULES_CLEAN += module_pip_clean

MODULES_LIBS += libpip.a
endif

ifeq ($(KIVA_MODULE_FACEAE), y)
	CFLAGS += -DMODULE_FACEAE_ENABLE
	FACEDET_ENABLE=y
	MODULES += module_faceae
	MODULES_CLEAN += module_faceae_clean

	MODULES_LIBS += libfaceAE.a
endif
# audio_ns
ifeq ($(KIVA_MERT), y)
	CFLAGS += -DMERT_ENABLE
	ifeq ($(findstring T40, $(KIVA_ENV_HOST)), T40)
		LIBS += -L$(KIVA_ENV_LIBS_DIR)/audio_ns/t40/lib/$(KIVA_ENV_LIBC_TYPE) -lmert
	        LIBS += -L$(KIVA_ENV_LIBS_DIR)/libnna/t40/$(KIVA_ENV_LIBC_TYPE) -lvenus

                INCLUDES += -I$(KIVA_ENV_LIBS_DIR)/audio_ns/t40/include

	else ifeq ($(findstring T41, $(KIVA_ENV_HOST)), T41)
		LIBS += -L$(KIVA_ENV_LIBS_DIR)/audio_ns/t41/lib/$(KIVA_ENV_LIBC_TYPE) -lmert
		LIBS += -L$(KIVA_ENV_LIBS_DIR)/libnna/t41/$(KIVA_ENV_LIBC_TYPE) -lvenus -ldrivers -laip

		INCLUDES += -I$(KIVA_ENV_LIBS_DIR)/audio_ns/t41/include
	endif
endif

# module remote
ifeq ($(KIVA_MODULE_REMOTE), y)
	CFLAGS += -DMODULE_REMOTE_ENABLE

	INCLUDES += -I$(KIVA_ENV_LIBS_DIR)/../src/modules/module_ucamera/
	MODULES += module_remote
	MODULES_CLEAN += module_remote_clean

	MODULES_LIBS += libremote.a
endif

# module optzoom
ifeq ($(KIVA_MODULE_OPTZOOM), y)
	CFLAGS += -DMODULE_OPTZOOM_ENABLE

	MODULES += module_optzoom
	MODULES_CLEAN += module_optzoom_clean

	MODULES_LIBS += liboptzoom.a
endif

# facedet enable
ifeq ($(FACEDET_ENABLE), y)
	ifeq ($(findstring T31, $(KIVA_ENV_HOST)), T31)
		LIBS += -L$(KIVA_ENV_LIBS_DIR)/faceDet/lib/$(KIVA_ENV_LIBC_TYPE) -lfaceDet_inf -ljzdl
		LIBS += -L$(KIVA_ENV_LIBS_DIR)/mxu/lib/$(KIVA_ENV_LIBC_TYPE) -lmxu_core -lmxu_imgproc -lmxu_contrib -lmxu_merge -lmxu_objdetect -lmxu_video

		INCLUDES += -I$(KIVA_ENV_LIBS_DIR)/faceDet/include
	else ifeq ($(findstring T40, $(KIVA_ENV_HOST)), T40)
		LIBS += -L$(KIVA_ENV_LIBS_DIR)/facedet/t40/lib/$(KIVA_ENV_LIBC_TYPE) -lfaceCap_inf
	        LIBS += -L$(KIVA_ENV_LIBS_DIR)/libnna/t40/$(KIVA_ENV_LIBC_TYPE) -lvenus

		INCLUDES += -I$(KIVA_ENV_LIBS_DIR)/facedet/t40/include

	else ifeq ($(findstring T41, $(KIVA_ENV_HOST)), T41)
		LIBS += -L$(KIVA_ENV_LIBS_DIR)/facedet/t41/lib/$(KIVA_ENV_LIBC_TYPE) -lfaceCap_inf
		LIBS += -L$(KIVA_ENV_LIBS_DIR)/libnna/t41/$(KIVA_ENV_LIBC_TYPE) -lvenus -ldrivers -laip

		INCLUDES += -I$(KIVA_ENV_LIBS_DIR)/facedet/t41/include
	endif

endif

