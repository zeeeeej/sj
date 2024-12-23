#!/bin/bash

# 设置错误时退出
set -e

# 设置交叉编译器路径
GCC_PATH="/home/lili/code/guanjian/t23_cam/ISVP-T23-1.1.2-20240204/software/zh/Ingenic-SDK-T23-1.1.2-20240204-zh/resource/toolchain/gcc_540/mips-gcc540-glibc222-64bit-r3.3.0.smaller/bin"

# 设置编译环境
export PATH=${GCC_PATH}:$PATH
export CROSS_COMPILE=mips-linux-gnu-
export CC=${CROSS_COMPILE}gcc
export CXX=${CROSS_COMPILE}g++

# 设置编译标志
export CFLAGS="-Os -Wall -march=mips32r2 -muclibc"
export LDFLAGS="-static -muclibc"

echo "编译环境配置："
echo "GCC_PATH = $GCC_PATH"
echo "CC = $CC"
echo "CROSS_COMPILE = $CROSS_COMPILE"
echo "CFLAGS = $CFLAGS"

# 清理之前的配置
make distclean || true

# 配置
./configure --host=mips-linux-gnu \
    --prefix=/usr \
    --disable-shared \
    --enable-static \
    --disable-nls \
    --disable-doc \
    --disable-scripts

# 编译
make -j4

# 创建输出目录
mkdir -p output
cp src/xz/xz output/

# strip 减小文件体积
${CROSS_COMPILE}strip output/xz

echo "编译完成！"
echo "可执行文件位置: output/xz"
ls -lh output/xz