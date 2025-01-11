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
)

target_include_directories(Cam485Protocol_lib PUBLIC ${CMAKE_SOURCE_DIR}/Cam485Protocol
${CMAKE_SOURCE_DIR}/Peripheral/uart)



target_link_libraries(Cam485Protocol_lib 

    Peripheral_lib 

)
