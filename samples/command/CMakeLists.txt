add_library(cli_command_lib STATIC
    command/cli_command.c
)

target_include_directories(cli_command_lib PUBLIC command)
