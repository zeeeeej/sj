#ifndef MY_CLI_H
#define MY_CLI_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

/* 命令最大长度 */
#define MAX_CMD_LENGTH     128
/* 最大参数数量 */
#define MAX_ARGV_NUM       16
/* 支持的最大内部命令数 */
#define MAX_INTERNAL_CMD   10

/* 命令处理函数类型定义 */
typedef void (*cmd_handler)(int argc, char **argv);

/* 核心API声明 */
void register_command(const char *name, cmd_handler handler);  // 注册新命令
void command_loop(void);                                        // 启动命令行循环

/* 内置命令声明（可选暴露）*/
void exit_cmd(int argc, char **argv);  // 退出命令声明
void help_cmd(int argc, char **argv);  // 帮助命令声明
int command_init(void);
#endif /* MY_CLI_H */