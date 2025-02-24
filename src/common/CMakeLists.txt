add_library(src_common_lib STATIC
    load_config.c
    imp/imp_audio_ctl.c
    imp/imp_common.c
    # imp/imp_ipu_osd.c
    # imp/imp_ipu_osd.h
    # imp/imp_isp.h
    # imp/imp_isp.c
    # imp/imp_isposd.h
    
    # imp/imp_isposd.c
    imp/imp_video_ctl.c
    imp/imp-common.h
)


target_include_directories(src_common_lib PUBLIC ${CMAKE_SOURCE_DIR}/Peripheral/usb
    ${CMAKE_SOURCE_DIR}/Peripheral/uart
    ${CMAKE_SOURCE_DIR}/../src/common/include
)

# link_directories( ${CMAKE_SOURCE_DIR}/../src/common/libs)
# target_link_libraries(src_common_lib PRIVATE png)