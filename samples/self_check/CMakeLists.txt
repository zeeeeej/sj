add_library(self_check_lib STATIC
    cam_self_check.c
    cam_self_check.h
    gyro_self_check.c
    gyro_self_check.h
    self_check.c
    self_check.h
)

target_include_directories(self_check_lib PUBLIC ${CMAKE_SOURCE_DIR}/Peripheral/mpu)