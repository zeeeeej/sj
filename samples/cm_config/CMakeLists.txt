add_library(CM_Config_lib STATIC
    cm_config.c
    cm_config.h
    ParseIni.c
    ParseIni.h
)

target_include_directories(CM_Config_lib PUBLIC ${CMAKE_SOURCE_DIR}/CM_Config
    ${CMAKE_SOURCE_DIR}/log/include
)



