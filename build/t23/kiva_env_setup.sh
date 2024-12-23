#########################################################################
# File Name: kiva_env_setup.sh
# Author: Tania Xiang
# mail: xiuhui.xiang@ingenic.com
# Created Time: 2022-12-01 10:29
#########################################################################
#!/bin/bash

if [ -f build/t23/kiva_env_setup.sh ]; then
	echo "Source Kiva build environment!"
	# host : T23N
	export KIVA_ENV_HOST=T23N

	#platform
	export KIVA_ENV_TOP=$(pwd)
	export KIVA_ENV_PLATFORM=/home/lili/code/guanjian/t23_cam/ISVP-T23-1.1.2-20240204/software/zh/Ingenic-SDK-T23-1.1.2-20240204-zh/opensource
	export KIVA_ENV_UBOOT_DIR=${KIVA_ENV_PLATFORM}/uboot-t23
	# kernel version : 3.10 / 4.4
	export KIVA_ENV_KERNEL_DIR=${KIVA_ENV_PLATFORM}/kernel
	export KIVA_ENV_DRIVERS_DIR=${KIVA_ENV_PLATFORM}/drivers-t23
	export KIVA_ENV_SDK_DIR=~/work/isvp/proj/sdk-lv3-t23

	# application
	if [ ${KIVA_ENV_HOST:0:3} = "T23" ]; then

		export KIVA_ENV_UCAMERA_DRIVERS_DIR=${KIVA_ENV_TOP}/driver/usbcamera/3.10
		#gcc version
		export KIVA_ENV_GCC_PATH=/home/lili/code/guanjian/t23_cam/ISVP-T23-1.1.2-20240204/software/zh/Ingenic-SDK-T23-1.1.2-20240204-zh/resource/toolchain/gcc_540/mips-gcc540-glibc222-64bit-r3.3.0.smaller/bin
	fi
	export KIVA_ENV_SRC_DIR=${KIVA_ENV_TOP}/src
	export KIVA_ENV_LIBS_DIR=${KIVA_ENV_TOP}/libs
	export KIVA_ENV_UTILS_DIR=${KIVA_ENV_TOP}/utils
	export KIVA_ENV_MODULES_DIR=${KIVA_ENV_SRC_DIR}/modules

	# libc type : glic / uclibc
	export KIVA_ENV_LIBC_TYPE=uclibc

	# sdk version
	export KIVA_ENV_VERSION=V1.1.0

	echo KIVA_ENV_TOP=${KIVA_ENV_TOP}
	echo KIVA_ENV_PLATFORM=${KIVA_ENV_PLATFORM}
	echo KIVA_ENV_UBOOT_DIR=${KIVA_ENV_UBOOT_DIR}
	echo KIVA_ENV_KERNEL_DIR=${KIVA_ENV_KERNEL_DIR}
	echo KIVA_ENV_DRIVERS_DIR=${KIVA_ENV_DRIVERS_DIR}
	echo KIVA_ENV_SDK_DIR=${KIVA_ENV_SDK_DIR}
	echo KIVA_ENV_UCAMERA_DRIVERS_DIR=${KIVA_ENV_UCAMERA_DRIVERS_DIR}
	echo KIVA_ENV_SRC_DIR=${KIVA_ENV_SRC_DIR}
	echo KIVA_ENV_LIBS_DIR=${KIVA_ENV_LIBS_DIR}
	echo KIVA_ENV_UTILS_DIR=${KIVA_ENV_UTILS_DIR}
	echo KIVA_ENV_MODULES_DIR=${KIVA_ENV_MODULES_DIR}
	echo KIVA_ENV_LIBC_TYPE=${KIVA_ENV_LIBC_TYPE}
	echo KIVA_ENV_VERSION=${KIVA_ENV_VERSION}
	echo KIVA_ENV_GCC_PATH=${KIVA_ENV_GCC_PATH}
else
	echo "please run source at top of kiva!"
fi
