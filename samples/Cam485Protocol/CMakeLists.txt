add_library(Cam485Protocol_lib STATIC
    circular_buffer.c
    circular_buffer.h
    Cam485Protocol.h
    Cam485Protocol.c
    Cam485ProtocolCommon.h
    MsgDispatcher.c
    MsgDispatcher.h
    MsgDispatherPort.c
    MsgDispatherPort.h
    CommonMsgTable.c
    CommonMsgTable.h
    SID01_Heartbeat.c
    SID01_Heartbeat.h
    SID02_QueryAttribute.h
    SID02_QueryAttribute.c
    SID03_SetAttribute.h
    SID03_SetAttribute.c
    SID04_RestoreFactorySettings.h
    SID04_RestoreFactorySettings.c
    SID05_ResetCamera.h
    SID05_ResetCamera.c
    SID06_ActiveScreenshot.h
    SID06_ActiveScreenshot.c
    SID07_QueryPhotoInfo.c
    SID07_QueryPhotoInfo.h
    SID08_DeletePicture.c
    SID08_DeletePicture.h
    SID09_GetPhoto.c
    SID09_GetPhoto.h
    SID0A_PhotoGetComplete.h
    SID0A_PhotoGetComplete.c
    SID0B_FirmwareUpdateNotice.h
    SID0B_FirmwareUpdateNotice.c
    SID0C_SendFirmwareUpdatePacket.h
    SID0C_SendFirmwareUpdatePacket.c
    SID0D_DoorOpenOrCloseSignal.h
    SID0D_DoorOpenOrCloseSignal.c
    ImageInfoList.c
    ImageInfoList.h
    md5.c
    md5.h
    ParseIni.h
    ParseIni.c
    cm_config.h
    cm_config.c
)

target_include_directories(Cam485Protocol_lib PUBLIC ${CMAKE_SOURCE_DIR}/Cam485Protocol
    ${CMAKE_SOURCE_DIR}/Peripheral/uart
    ${CMAKE_SOURCE_DIR}/Peripheral/camera
    ${CMAKE_SOURCE_DIR}/log/include
)



target_link_libraries(Cam485Protocol_lib 
    Peripheral_lib 
    log_lib
)
