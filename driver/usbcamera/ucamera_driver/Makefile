# 定义工具链完整路径
TOOLCHAIN_PATH := /home/lili/code/guanjian/t23_cam/ISVP-T23-1.1.2-20240204/software/zh/Ingenic-SDK-T23-1.1.2-20240204-zh/resource/toolchain/gcc_540/mips-gcc540-glibc222-64bit-r3.3.0.smaller/bin

# 设置交叉编译器
CROSS_COMPILE ?= $(TOOLCHAIN_PATH)/mips-linux-gnu-

KDIR := ${KIVA_ENV_KERNEL_DIR}

MODULE_NAME := usbcamera

# 添加调试打印
$(info ========== Build Environment ==========)
$(info TOOLCHAIN_PATH = $(TOOLCHAIN_PATH))
$(info CROSS_COMPILE = $(CROSS_COMPILE))
$(info KDIR = $(KDIR))
$(info Current Directory = $(shell pwd))
$(info ===================================)

# 检查工具链是否存在
$(shell if [ ! -f "$(CROSS_COMPILE)gcc" ]; then \
    echo "ERROR: Compiler not found at $(CROSS_COMPILE)gcc"; \
    exit 1; \
fi)

INCLUDE = -I$(KDIR)/include/generated/ \
          -I$(PWD)/include

EXTRA_CFLAGS += -Wall 
EXTRA_CFLAGS += $(INCLUDE)

$(MODULE_NAME)-objs := libuac.a libuvc.a libcore.a ucam_driver.o libimd.a
$(MODULE_NAME)-objs += debug/adb_driver/f_adb.o

obj-m += $(MODULE_NAME).o

all: check_env modules

check_env:
	@echo "Checking build environment..."
	@if [ ! -f "$(CROSS_COMPILE)gcc" ]; then \
		echo "ERROR: Compiler not found at $(CROSS_COMPILE)gcc"; \
		exit 1; \
	fi
	@echo "Compiler found: $(CROSS_COMPILE)gcc"
	@echo "Kernel directory: $(KDIR)"

modules:
	@echo "Building modules with CROSS_COMPILE=$(CROSS_COMPILE)"
	$(MAKE) -C $(KDIR) \
		CROSS_COMPILE=$(CROSS_COMPILE) \
		ARCH=mips \
		M=$(shell pwd) \
		modules

clean:
	find . -name "*.o" | xargs rm -vf
	find . -name ".cmd" | xargs rm -vf
	find . -name "\.*cmd" | xargs rm -vf
	@rm -rf *.o *~ .depend .*.cmd  *.mod.c .tmp_versions *.ko *.symvers modules.order

.PHONY: all clean check_env modules