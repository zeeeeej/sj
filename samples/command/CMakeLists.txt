set(cmd_src_dir ${CMAKE_SOURCE_DIR}/command)


add_library(cli_command_lib STATIC 
    ${cmd_src_dir}/cli_command.c
    ${cmd_src_dir}/cli_command.h

)

target_include_directories(cli_command_lib PUBLIC
    ${cmd_src_dir}/
)