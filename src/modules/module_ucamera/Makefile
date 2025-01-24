
####################################
# CC CPP AR
# CFLAGS INCLUDES
# come from top of Makefile in Kiva
###################################

OBJS := module_ucamera_control.o \
	uvc_control.o \
	uac_control.o \
	xu_control/uvc_eu_control.o

INCLUDES += -I$(KIVA_ENV_MODULES_DIR)/module_ucamera/xu_control/

TARGET := libucamera.a

%.o:%.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ -c $^

%.o:%.cpp
	$(CPP) $(CFLAGS) $(CPPFLAGS) $(INCLUDES) -o $@ -c $^

%.o:%.cc
	$(CPP) $(CFLAGS) $(INCLUDES) -lstdc++ -o $@ -c $^

$(info "ucamera makefile target")
$(TARGET): $(OBJS)
	$(AR) $(ARFLAG) $@ $^
	cp $(TARGET) ../

All: $(TARGET)
clean:
	rm -f $(TARGET) $(OBJS)

#INCLUDES += -I$(KIVA_ENV_MODULES_DIR)/module_ucamera/xu_control/

#OBJS += $(KIVA_ENV_MODULES_DIR)/module_ucamera/module_ucamera_control.o \
	#$(KIVA_ENV_MODULES_DIR)/module_ucamera/xu_control/uvc_eu_control.o\
	#$(KIVA_ENV_MODULES_DIR)/module_ucamera/xu_control/uvc_xu_control.o\
	#$(KIVA_ENV_MODULES_DIR)/module_ucamera/uac_control.o \
	#$(KIVA_ENV_MODULES_DIR)/module_ucamera/uvc_control.o

