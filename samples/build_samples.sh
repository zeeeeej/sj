#!/bin/bash

TOOLCHAIN_ROOT=$(realpath "$(dirname "$0")/../toolchain/gcc_540/mips-gcc540-uclibc0.9.33.2-64bit-r3.3.0.smaller")

PROJECT_DIR=$(pwd)

# 构建目录
BUILD_DIR="$PROJECT_DIR/build"

# 生成构建目录，如果不存在
if [ ! -d "$BUILD_DIR" ]; then
    echo "Creating build directory..."
    mkdir "$BUILD_DIR"
fi

cd "$BUILD_DIR"

# 设置 CMake 编译器和工具链,版本:uclibc0.9.33.2-64bit-r3.3.0.smaller
export PATH="$TOOLCHAIN_PATH/bin:$PATH"
export CC="$TOOLCHAIN_PATH/bin/mips-linux-gnu-gcc"
export CXX="$TOOLCHAIN_PATH/bin/mips-linux-gnu-g++"

CMAKE_BUILD_TYPE="Release"  # 可以修改为 Debug

echo "Running cmake configuration..."
cmake -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE -DCMAKE_TOOLCHAIN_FILE="$PROJECT_DIR/toolchain.cmake" $PROJECT_DIR

if [ $? -ne 0 ]; then
    echo "CMake configuration failed!"
    exit 1
fi

echo "Building the project..."
make -j$(nproc)


if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

echo "Build completed successfully!"
