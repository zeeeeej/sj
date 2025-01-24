add_library(cli_command_lib STATIC 
    ${CMAKE_SOURCE_DIR}/command/cli_command.c
    ${CMAKE_SOURCE_DIR}/command/cli_command.h
)

target_include_directories(cli_command_lib PUBLIC ${CMAKE_SOURCE_DIR}/command/)
