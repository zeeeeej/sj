cmake_minimum_required(VERSION 3.0)

project(samples)


# 在主CMakeLists.txt中定义全局包含目录变量
set(COMMON_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/door_detect
    ${CMAKE_SOURCE_DIR}/modbus_task
    ${CMAKE_SOURCE_DIR}/Peripheral
    ${CMAKE_SOURCE_DIR}/self_check
    ${CMAKE_SOURCE_DIR}/utils
    ${CMAKE_SOURCE_DIR}/Cam485Protocol
    CACHE INTERNAL "Common include directories"  
)

include_directories(COMMON_INCLUDE_DIRS)

# 添加子目录
add_subdirectory(utils)
add_subdirectory(door_detect)
add_subdirectory(modbus_task)
add_subdirectory(Peripheral)
add_subdirectory(self_check)
add_subdirectory(Cam485Protocol)


add_executable(sample_camera_rst
    sample_camera_rst.c
)

link_directories(../libs/t23_sdk/lib/uclibc)



