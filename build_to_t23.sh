#!/bin/bash

# 设置错误时退出
set -e

# 手动设置交叉编译器路径
GCC_PATH="/home/lili/code/guanjian/t23_cam/ISVP-T23-1.1.2-20240204/software/zh/Ingenic-SDK-T23-1.1.2-20240204-zh/resource/toolchain/gcc_540/mips-gcc540-glibc222-64bit-r3.3.0.smaller/bin"

# 设置编译环境
export PATH=${GCC_PATH}:$PATH
export CROSS_COMPILE=mips-linux-gnu-
export CC=${GCC_PATH}/${CROSS_COMPILE}gcc
export CXX=${GCC_PATH}/${CROSS_COMPILE}g++
export CFLAGS="-Os"  # 优化大小
export LDFLAGS="-static"  # 静态链接

echo "编译环境配置："
echo "GCC_PATH = $GCC_PATH"
echo "CC = $CC"
echo "CROSS_COMPILE = $CROSS_COMPILE"
echo "CFLAGS = $CFLAGS"

# 检查源码目录是否存在
if [ ! -d "xz-5.4.5" ]; then
    echo "错误：找不到 xz-5.4.5 目录"
    exit 1
fi

# 进入源码目录
cd xz-5.4.5

# 配置
echo "配置编译环境..."
./configure --host=mips-linux-gnu \
    --prefix=/usr \
    --disable-shared \
    --enable-static \
    --disable-nls \
    --disable-scripts \
    --disable-doc

# 编译
echo "开始编译..."
make -j4

# 创建输出目录
mkdir -p ../output
cp src/xz/xz ../output/

echo "编译完成！"
echo "可执行文件位置: output/xz"

# 显示文件信息
file ../output/xz

# strip 减小文件体积
${CROSS_COMPILE}strip ../output/xz

echo "最终文件大小："
ls -lh ../output/xz 