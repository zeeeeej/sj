add_library(utils_lib STATIC
    cJSON.h
    cJSON.c
    cm_base64.h
    cm_base64.c
    cm_common.h
    cm_conf.c
    cm_conf.h
    cm_list.c
    cm_list.h
    cm_utils.c
    cm_utils.h
    jkbytes.c
    jkbytes.h
)

target_include_directories(utils_lib PUBLIC ${CMAKE_SOURCE_DIR})