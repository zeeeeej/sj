set(cmd_src_dir ${CMAKE_SOURCE_DIR}/command)


add_library(cli_command_lib STATIC 
    ${cmd_src_dir}/cli.c
    ${cmd_src_dir}/cli.h
    ${cmd_src_dir}/extern_cli.c
    ${cmd_src_dir}/extern_cli.h

)

target_include_directories(cli_command_lib PUBLIC
    ${cmd_src_dir}/
    ${CMAKE_SOURCE_DIR}/Cam485Protocol
)