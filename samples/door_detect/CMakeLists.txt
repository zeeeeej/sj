add_library(door_detect_lib STATIC
    door_detect.h
    door_detect.c
)

target_include_directories(door_detect_lib PUBLIC ${CMAKE_SOURCE_DIR}/Peripheral/mpu
    ${CMAKE_SOURCE_DIR}/log/include
)