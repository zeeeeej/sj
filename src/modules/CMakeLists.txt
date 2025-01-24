add_library(src_modules_lib STATIC
    module_config/module_config.c
    module_ucamera/uac_control.h
    module_ucamera/uac_control.c
    module_ucamera/uvc_control.c
    module_ucamera/uvc_control.h
    module_ucamera/xu_control/uvc_eu_control.h
    module_ucamera/xu_control/uvc_eu_control.c
    module_ucamera/xu_control/uvc_xu_control.c
    module_ucamera/xu_control/uvc_xu_control.h
    module_ucamera/module_ucamera_control.c
    
)


target_include_directories(src_modules_lib PUBLIC ${CMAKE_SOURCE_DIR}/Peripheral/usb
    ${CMAKE_SOURCE_DIR}/../src/include
    ${CMAKE_SOURCE_DIR}/../src/common/imp
)