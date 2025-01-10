cmake_minimum_required(VERSION 3.0)

project(samples)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)


set (3dparty_DIR ${CMAKE_SOURCE_DIR}/../libs)
set (SRC_DIR ${CMAKE_SOURCE_DIR}/../src)


# 指定交叉编译器路径和相关配置, 交叉编译器的是uclibc版本的
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR mips)

set(TOOLCHAIN_ROOT "/home/kxy/T23_Tool/ISVP-T23-1.1.2-20240204/software/zh/Ingenic-SDK-T23-1.1.2-20240204-zh/resource/toolchain/gcc_540/mips-gcc540-glibc222-64bit-r3.3.0.smaller")

set(CMAKE_C_COMPILER "${TOOLCHAIN_ROOT}/bin/mips-linux-gnu-gcc")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_ROOT}/bin/mips-linux-gnu-g++")
set(CMAKE_FIND_ROOT_PATH "${TOOLCHAIN_ROOT}")

# 设置搜索路径
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)


# 在主CMakeLists.txt中定义全局包含目录变量
set(COMMON_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/door_detect
    ${CMAKE_SOURCE_DIR}/modbus_task
    ${CMAKE_SOURCE_DIR}/Peripheral
    ${CMAKE_SOURCE_DIR}/self_check
    ${CMAKE_SOURCE_DIR}/utils
    ${CMAKE_SOURCE_DIR}/tmp
    ${CMAKE_SOURCE_DIR}/log
    ${CMAKE_SOURCE_DIR}/command
    ${CMAKE_SOURCE_DIR}/Cam485Protocol
    ${CMAKE_SOURCE_DIR}/Peripheral/camera
    ${CMAKE_SOURCE_DIR}/Peripheral/wdt
    ${3dparty_DIR}/t23_sdk/include
    ${3dparty_DIR}/modbus/include
    ${3dparty_DIR}/ucamcore/include
    ${3dparty_DIR}/anticopy/include
    ${SRC_DIR}/include
    ${SRC_DIR}/common/imp
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/../src/common/include
    ${CMAKE_SOURCE_DIR}/../src/include
    CACHE INTERNAL "Common include directories"  
)

include_directories(COMMON_INCLUDE_DIRS)
include_directories(${COMMON_INCLUDE_DIRS})

link_directories(${3dparty_DIR}/t23_sdk/lib/uclibc)

# find_library(MODBUS_LIB modbus PATHS ${3dparty_DIR}/modbus/lib)
# message(STATUS "Modbus Library: ${MODBUS_LIB}")

add_library(samples_lib STATIC cm_config.h cm_config.c)


# 添加子目录
message(STATUS "Adding subdirectory: ${CMAKE_SOURCE_DIR}/../src/common")
add_subdirectory(${CMAKE_SOURCE_DIR}/../src/common ${CMAKE_BINARY_DIR}/common_build)
add_subdirectory(${CMAKE_SOURCE_DIR}/../src/modules ${CMAKE_BINARY_DIR}/modules_build)
add_subdirectory(log)
add_subdirectory(tmp)
add_subdirectory(command)
add_subdirectory(utils)
add_subdirectory(Peripheral)
add_subdirectory(door_detect)
add_subdirectory(modbus_task)
add_subdirectory(self_check)
add_subdirectory(Cam485Protocol)
message(STATUS "Building utils_lib in ${CMAKE_CURRENT_SOURCE_DIR}")


add_executable(sample_camera_rst
    sample_camera_rst.c
)

target_link_libraries(sample_camera_rst utils_lib door_detect_lib modbus_task_lib Peripheral_lib self_check_lib Cam485Protocol_lib)
target_include_directories(sample_camera_rst PRIVATE ${COMMON_INCLUDE_DIRS})
# 链接库文件
target_link_libraries(sample_camera_rst 
    utils_lib 
    door_detect_lib 
    modbus_task_lib 
    Peripheral_lib 
    self_check_lib 
    Cam485Protocol_lib
    log_lib
    tmp_lib
    cli_command_lib
    src_common_lib
    samples_lib
    pthread
    alog
    imp
    sysutils
    rt
    src_modules_lib
    ${3dparty_DIR}/ucamcore/lib/uclibc/libusbcamera.a
    ${3dparty_DIR}/modbus/lib/libmodbus.a
)


