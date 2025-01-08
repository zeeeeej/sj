add_library(tmp_lib STATIC
    file_trans.c
    file_trans.h
    mem_pool.c
    mem_pool.h
    MessageDispatcher.c
    MessageDispatcher.h 
    MessageDispaterPort.c
    MessageDispaterPort.h
    modbus_master.c
    modbus_master.h
    modbus_task.c
    modbus_task.h
    ota_process.c
    ota_process.h
    self_check_process.c
    self_check_process.h
    system_list.c
    take_photo.c
    take_photo.h
    trans_door_image.c
    trans_door_image.h
)

target_include_directories(tmp_lib PUBLIC ${CMAKE_SOURCE_DIR}/Peripheral/uart)