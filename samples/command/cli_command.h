#ifndef CLI_COMMAND_H
#define CLI_COMMAND_H

#include <stddef.h>

// 命令处理函数类型
typedef int (*cli_cmd_func)(int argc, char *argv[]);

// 命令结构体
typedef struct cli_command {
    const char *name;         // 命令名称
    const char *help;         // 帮助信息
    cli_cmd_func func;        // 命令处理函数
    struct cli_command *next; // 链表下一个节点
} cli_command_t;

// 注册命令
int cli_register_command(const cli_command_t *cmd);

// 执行命令
int cli_execute_command(const char *cmdline);

// 显示所有命令
void cli_show_commands(void);

// 初始化命令行接口
int cli_init(void);

// 启动命令行接口
int cli_start(void);

// 停止命令行接口
void cli_stop(void);

#endif // CLI_COMMAND_H