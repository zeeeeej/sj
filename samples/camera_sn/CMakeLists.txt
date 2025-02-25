add_library(CameraSn STATIC
    camera_sn.c
    camera_sn.h
)

target_include_directories(CameraSn PUBLIC ${CMAKE_SOURCE_DIR}/camera_sn
    ${CMAKE_SOURCE_DIR}/log/include
)



