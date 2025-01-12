add_library(Cam485Protocol_lib STATIC
    # CommonInterface.h
    Heartbeat.c
    Heartbeat.h
    SendPhoto.c
    SendPhoto.h

    MsgDispatcher.c
    MsgDispatcher.h
    MsgDispatherPort.c
    MsgDispatherPort.h
    CommonMsgTable.c
    CommonMsgTable.h
    SID06_ActiveScreenshot.h
    SID06_ActiveScreenshot.c
    ProtocolPort.h
    ProtocolPort.c
    SID07_QueryPhotoInfo.c
    SID07_QueryPhotoInfo.h
    SID09_GetPhoto.c
    SID09_GetPhoto.h
    ProtocolPort.c
    ProtocolPort.h
    ImageInfoList.c
    ImageInfoList.h
    md5.c
    md5.h
)

target_include_directories(Cam485Protocol_lib PUBLIC ${CMAKE_SOURCE_DIR}/Cam485Protocol
${CMAKE_SOURCE_DIR}/Peripheral/uart)



target_link_libraries(Cam485Protocol_lib 

    Peripheral_lib 

)
