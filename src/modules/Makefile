####################################
# CC CPP AR
# CFLAGS INCLUDES
# come from top of Makefile in Kiva
###################################
include kiva_modules.mk

TARGET := libmodules.a

.PHONY: clean $(MODULES) $(MODULES_CLEAN)

All: $(TARGET)

# ar -rcsT (T: make a thin archive)
$(TARGET): $(MODULES)
	$(AR) -rcsT $@ $(MODULES_LIBS)

module_uvc_xu:
	make -C $(KIVA_ENV_MODULES_DIR)/module_ucamera/xu_control/

module_uvc_xu_clean:
	make -C $(KIVA_ENV_MODULES_DIR)/module_ucamera/xu_control clean

module_ucamera:
	make -C $(KIVA_ENV_MODULES_DIR)/module_ucamera/

module_ucamera_clean:
	make -C $(KIVA_ENV_MODULES_DIR)/module_ucamera clean

module_config:
	make -C $(KIVA_ENV_MODULES_DIR)/module_config/

module_config_clean:
	make -C $(KIVA_ENV_MODULES_DIR)/module_config clean

module_led:
	make -C $(KIVA_ENV_MODULES_DIR)/module_led_control/

module_led_clean:
	make -C $(KIVA_ENV_MODULES_DIR)/module_led_control clean

module_autofocus:
	make -C $(KIVA_ENV_MODULES_DIR)/module_autofocus/

module_autofocus_clean:
	make -C $(KIVA_ENV_MODULES_DIR)/module_autofocus clean

module_facezoom:
	make -C $(KIVA_ENV_MODULES_DIR)/module_facezoom/

module_facezoom_clean:
	make -C $(KIVA_ENV_MODULES_DIR)/module_facezoom clean

module_pip:
	make -C $(KIVA_ENV_MODULES_DIR)/module_pip/

module_pip_clean:
	make -C $(KIVA_ENV_MODULES_DIR)/module_pip clean

module_remote:
	make -C $(KIVA_ENV_MODULES_DIR)/module_remote/

module_remote_clean:
	make -C $(KIVA_ENV_MODULES_DIR)/module_remote clean

module_optzoom:
	make -C $(KIVA_ENV_MODULES_DIR)/module_optzoom/

module_optzoom_clean:
	make -C $(KIVA_ENV_MODULES_DIR)/module_optzoom clean

module_faceae:
	make -C $(KIVA_ENV_MODULES_DIR)/module_faceAE

module_faceae_clean:
	make -C $(KIVA_ENV_MODULES_DIR)/module_faceAE clean

clean:	$(MODULES_CLEAN)
	rm -f $(TARGET)
	rm -rf $(MODULES_LIBS)
