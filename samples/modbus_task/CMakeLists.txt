add_library(modbus_task_lib STATIC
    md5.h
    md5.c
    wind_connect_up.h
    wind_connect_up.c
    wind_connect.c
    wind_connect.h
    wind_modbus_wrap.c
    wind_modbus_wrap.h
    wind_process_manager.c
    wind_process_manager.h
    wind_protocol_wrap_up.c
    wind_protocol_wrap_up.h
    wind_protocol_wrap.c
    wind_protocol_wrap.h
)

target_include_directories(modbus_task_lib PUBLIC ${CMAKE_SOURCE_DIR}/Peripheral/usb
${CMAKE_SOURCE_DIR}/Peripheral/uart

)