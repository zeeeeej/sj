cmake_minimum_required(VERSION 3.0)

project(samples)

set (3dparty_DIR ${CMAKE_SOURCE_DIR}/../libs)
set (SRC_DIR ${CMAKE_SOURCE_DIR}/../src)

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
    CACHE INTERNAL "Common include directories"  
)

include_directories(COMMON_INCLUDE_DIRS)
include_directories(${COMMON_INCLUDE_DIRS})


find_library(MODBUS_LIB modbus PATHS ${3dparty_DIR}/modbus/lib)
message(STATUS "Modbus Library: ${MODBUS_LIB}")



# 添加子目录
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

link_directories(../libs/t23_sdk/lib/uclibc)
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
    ${MODBUS_LIB}
    pthread
)


