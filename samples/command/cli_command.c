#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include "cli_command.h"
#include "circular_log.h"
#include <limits.h>  
#define CLI_TAG "CLI"
#define MAX_ARGS 16
#define MAX_CMD_LEN 256
#define CLI_PROMPT "T23> "

static cli_command_t *cmd_list = NULL;
static pthread_mutex_t cmd_mutex = PTHREAD_MUTEX_INITIALIZER;
static volatile int cli_running = 0;
static pthread_t cli_thread;

// 内置的帮助命令
static int cmd_help(int argc, char *argv[]) {
    printf("\nAvailable commands:\n");
    printf("------------------\n");
    cli_show_commands();
    return 0;
}

// 内置的退出命令
static int cmd_exit(int argc, char *argv[]) {
    cli_stop();
    return 0;
}

// 注册内置命令
static void register_builtin_commands(void) {
    static cli_command_t help_cmd = {
        .name = "help",
        .help = "Show available commands",
        .func = cmd_help
    };

    static cli_command_t exit_cmd = {
        .name = "exit",
        .help = "Exit the CLI",
        .func = cmd_exit
    };

    cli_register_command(&help_cmd);
    cli_register_command(&exit_cmd);
}

// 解析命令行
static int parse_command_line(char *cmdline, char *argv[], int max_args) {
    int argc = 0;
    char *token = strtok(cmdline, " \t\n\r");
    
    while (token && argc < max_args) {
        argv[argc++] = token;
        token = strtok(NULL, " \t\n\r");
    }
    
    return argc;
}

// 查找命令
static cli_command_t *find_command(const char *name) {
    cli_command_t *cmd = cmd_list;
    while (cmd) {
        if (strcmp(cmd->name, name) == 0) {
            return cmd;
        }
        cmd = cmd->next;
    }
    return NULL;
}

int cli_register_command(const cli_command_t *cmd) {
    if (!cmd || !cmd->name || !cmd->func) {
        return -1;
    }

    cli_command_t *new_cmd = malloc(sizeof(cli_command_t));
    if (!new_cmd) {
        return -1;
    }

    *new_cmd = *cmd;
    new_cmd->next = NULL;

    pthread_mutex_lock(&cmd_mutex);
    if (!cmd_list) {
        cmd_list = new_cmd;
    } else {
        cli_command_t *last = cmd_list;
        while (last->next) {
            last = last->next;
        }
        last->next = new_cmd;
    }
    pthread_mutex_unlock(&cmd_mutex);

    log_write(LOG_INFO, CLI_TAG, "Registered command: %s", cmd->name);
    return 0;
}

int cli_execute_command(const char *cmdline) {
    if (!cmdline || !*cmdline) {
        return 0;
    }

    char cmd_copy[MAX_CMD_LEN];
    strncpy(cmd_copy, cmdline, sizeof(cmd_copy) - 1);
    cmd_copy[sizeof(cmd_copy) - 1] = '\0';

    char *argv[MAX_ARGS];
    int argc = parse_command_line(cmd_copy, argv, MAX_ARGS);

    if (argc == 0) {
        return 0;
    }

    pthread_mutex_lock(&cmd_mutex);
    cli_command_t *cmd = find_command(argv[0]);
    pthread_mutex_unlock(&cmd_mutex);

    if (!cmd) {
        printf("Unknown command: %s\n", argv[0]);
        printf("Type 'help' for available commands\n");
        return -1;
    }

    log_write(LOG_DEBUG, CLI_TAG, "Executing command: %s", cmdline);
    return cmd->func(argc, argv);
}

void cli_show_commands(void) {
    pthread_mutex_lock(&cmd_mutex);
    cli_command_t *cmd = cmd_list;
    while (cmd) {
        printf("%-15s - %s\n", cmd->name, cmd->help ? cmd->help : "");
        cmd = cmd->next;
    }
    pthread_mutex_unlock(&cmd_mutex);
}

// 处理 cd 命令
static int handle_cd_command(const char *path) {
    if (chdir(path) != 0) {
        printf("cd: %s: %s\n", path, strerror(errno));
        return -1;
    }
    return 0;
}

static void *cli_thread_func(void *arg) {
    char cmdline[MAX_CMD_LEN];
    char current_dir[PATH_MAX];

    printf("\nWelcome to T23 Command Line Interface\n");
    printf("Type 'help' for available commands\n\n");

    while (cli_running) {
        // 显示当前路径
        if (getcwd(current_dir, sizeof(current_dir)) != NULL) {
            printf("%s %s", current_dir, CLI_PROMPT);
        } else {
            printf(CLI_PROMPT);
        }
        fflush(stdout);

        if (!fgets(cmdline, sizeof(cmdline), stdin)) {
            if (errno == EINTR) {
                continue;
            }
            break;
        }

        size_t len = strlen(cmdline);
        if (len > 0 && cmdline[len-1] == '\n') {
            cmdline[len-1] = '\0';
            len--;
        }

        if (len == 0) continue;

        // 解析命令
        char *argv[MAX_ARGS];
        int argc = parse_command_line(cmdline, argv, MAX_ARGS);
        
        if (argc == 0) continue;

        // 处理 cd 命令
        if (strcmp(argv[0], "cd") == 0) {
            if (argc > 1) {
                handle_cd_command(argv[1]);
            } else {
                handle_cd_command(getenv("HOME"));
            }
            continue;
        }

        // 查找并执行命令
        pthread_mutex_lock(&cmd_mutex);
        cli_command_t *cmd = find_command(argv[0]);
        pthread_mutex_unlock(&cmd_mutex);

        if (cmd) {
            cmd->func(argc, argv);
        } else {
            // 执行系统命令
            system(cmdline);
        }
    }

    return NULL;
}
int cli_init(void) {
    cmd_list = NULL;
    register_builtin_commands();
    return 0;
}

int cli_start(void) {
    if (cli_running) {
        return 0;
    }

    cli_running = 1;
    if (pthread_create(&cli_thread, NULL, cli_thread_func, NULL) != 0) {
        cli_running = 0;
        return -1;
    }

    log_write(LOG_INFO, CLI_TAG, "CLI started");
    return 0;
}

void cli_stop(void) {
    if (!cli_running) {
        return;
    }

    cli_running = 0;
    pthread_join(cli_thread, NULL);
    log_write(LOG_INFO, CLI_TAG, "CLI stopped");
}