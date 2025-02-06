#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#define MAX_CMD_LENGTH 128
#define MAX_ARGV_NUM 16
#define MAX_INTERNAL_CMD 10

/* 命令处理函数类型定义 */
typedef void (*cmd_handler)(int argc, char **argv);

/* 内部命令结构体 */
struct internal_cmd
{
    char *name;
    cmd_handler handler;
};

static struct internal_cmd cmd_list[MAX_INTERNAL_CMD];
static int cmd_count = 0;

/* 内置命令实现 */
void help_cmd(int argc, char **argv);
void exit_cmd(int argc, char **argv);

/* 命令注册函数 */
void register_command(const char *name, cmd_handler handler)
{
    if (cmd_count >= MAX_INTERNAL_CMD)
    {
        fprintf(stderr, "Command list full!\n");
        return;
    }

    cmd_list[cmd_count].name = strdup(name);
    cmd_list[cmd_count].handler = handler;
    cmd_count++;
}

/* 初始化内置命令 */
void init_builtin_commands(void)
{
    register_command("help", help_cmd);
    register_command("exit", exit_cmd);
}

/* 命令行解析函数 */
int parse_command(char *input, char **argv)
{
    int argc = 0;
    char *token = strtok(input, " \t\n\r");

    while (token != NULL && argc < MAX_ARGV_NUM - 1)
    {
        argv[argc++] = token;
        token = strtok(NULL, " \t\n\r");
    }
    argv[argc] = NULL;
    return argc;
}

/* 执行外部命令 */
void execute_external_cmd(char **argv)
{
    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork failed");
        return;
    }
    else if (pid == 0)
    { /* 子进程 */
        execvp(argv[0], argv);
        /* 如果execvp失败 */
        fprintf(stderr, "Command not found: %s\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    else
    { /* 父进程 */
        waitpid(pid, NULL, 0);
    }
}

/* 命令处理主循环 */
void* command_loop(void *arg)
{
    char input[MAX_CMD_LENGTH];
    char *argv[MAX_ARGV_NUM];

    while (1)
    {
        printf("MyCLI> ");
        fflush(stdout);

        /* 读取输入 */
        if (!fgets(input, MAX_CMD_LENGTH, stdin))
        {
            break; /* 遇到EOF */
        }

        /* 解析命令 */
        int argc = parse_command(input, argv);
        if (argc == 0)
            continue;

        /* 查找内部命令 */
        int found = 0;
        for (int i = 0; i < cmd_count; i++)
        {
            if (strcmp(argv[0], cmd_list[i].name) == 0)
            {
                cmd_list[i].handler(argc, argv);
                found = 1;
                break;
            }
        }

        /* 执行外部命令 */
        if (!found)
        {
            execute_external_cmd(argv);
        }
    }
}

/******************** 内置命令实现 ********************/
void help_cmd(int argc, char **argv)
{
    printf("Available commands:\n");
    for (int i = 0; i < cmd_count; i++)
    {
        printf("  %-10s\n", cmd_list[i].name);
    }
}

void exit_cmd(int argc, char **argv)
{
    printf("Exiting...\n");
    exit(EXIT_SUCCESS);
}

/******************** 主函数 ********************/

int command_init(void)
{
    init_builtin_commands();
    register_extern_module_commands();
    pthread_t command_loop_pid;
    pthread_create(&command_loop_pid, NULL, command_loop, NULL);
    return 0;
}