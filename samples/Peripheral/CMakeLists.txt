add_library(Peripheral_lib STATIC
    ${CMAKE_SOURCE_DIR}/Peripheral/camera/cm_video_ctrl_t23.c
    ${CMAKE_SOURCE_DIR}/Peripheral/camera/cm_video_ctrl.h
    ${CMAKE_SOURCE_DIR}/Peripheral/camera/cm_video_ctrl.c
    # ${CMAKE_SOURCE_DIR}/Peripheral/camera/sample-Change-Resolution.c
    ${CMAKE_SOURCE_DIR}/Peripheral/camera/sample-common.c
    ${CMAKE_SOURCE_DIR}/Peripheral/camera/sample-common.h
    ${CMAKE_SOURCE_DIR}/Peripheral/mpu/mpu_ctrl.c
    ${CMAKE_SOURCE_DIR}/Peripheral/mpu/mpu_ctrl.h
    ${CMAKE_SOURCE_DIR}/Peripheral/mpu/yq_mpu_impl_lsm6ds3trc.cpp
    ${CMAKE_SOURCE_DIR}/Peripheral/mpu/yq_mpu_impl_lsm6ds3trc.h
    ${CMAKE_SOURCE_DIR}/Peripheral/mpu/yq_mpu.h
    ${CMAKE_SOURCE_DIR}/Peripheral/uart/cm_uart.c
    ${CMAKE_SOURCE_DIR}/Peripheral/uart/cm_uart.h
    ${CMAKE_SOURCE_DIR}/Peripheral/wdt/wdt.c
    ${CMAKE_SOURCE_DIR}/Peripheral/wdt/wdt.h
    ${CMAKE_SOURCE_DIR}/Peripheral/pwm/pwm_ctrl.c
    ${CMAKE_SOURCE_DIR}/Peripheral/pwm/pwm_ctrl.h
)
target_link_libraries(Peripheral_lib PUBLIC 
    pthread
    log_lib
    CM_Config_lib
    # src_common_lib
)

target_include_directories(Peripheral_lib PUBLIC ${CMAKE_SOURCE_DIR}/Peripheral
    ${CMAKE_SOURCE_DIR}/cm_config
    ${CMAKE_SOURCE_DIR}/log/include
    ${CMAKE_SOURCE_DIR}/../src/common
)

