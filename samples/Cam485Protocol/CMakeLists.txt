add_library(Cam485Protocol_lib STATIC
    CommonInterface.h
    Heartbeat.c
    Heartbeat.h
    SendPhoto.c
    SendPhoto.h
)

target_include_directories(Cam485Protocol_lib PUBLIC ${CMAKE_SOURCE_DIR}/Cam485Protocol)
