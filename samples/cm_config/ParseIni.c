#include "ParseIni.h"
#include <stddef.h>
#include "elog.h"
Config global_config = {0};

char* trim(char* str)
{
    if (str == NULL) return NULL;  
    char *end;
    while (isspace((unsigned char)*str)) str++;  // 去掉前导空格
    if (*str == '\0') return str;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

int parse_ini()
{
    FILE *file = fopen(CONFIG_FILE, "r");
    if (!file) {
        log_e("Can't open file: %s", CONFIG_FILE);
        return -1;
    }

    char line[MAX_NAME_LENGTH];
    char current_section[MAX_NAME_LENGTH] = "";

    while (fgets(line, sizeof(line), file)) {
        char *trimmed_line = trim(line);

        // 忽略空行或注释行
        if (trimmed_line[0] == '\0' || trimmed_line[0] == ';' || trimmed_line[0] == '#') {
            continue;
        }

        // 检测是否为 section
        if (trimmed_line[0] == '[') {
            char *end = strchr(trimmed_line, ']');
            if (end) {
                *end = '\0';
                strncpy(current_section, trimmed_line + 1, MAX_NAME_LENGTH);
            }
        }
        // 解析键值对
        else 
        {
            char *equals = strchr(trimmed_line, '=');
            if (equals) {
                *equals = '\0';
                char *key = trim(trimmed_line);
                char *value = trim(equals + 1);
                save_to_config(current_section, key, value);
            }
        }
    }

    fclose(file);
    return 0;
}

const char *get_config_value(const char *section, const char *key)
{
    for (int i = 0; i < global_config.section_count; i++) 
    {
        if (strcmp(global_config.sections[i].section, section) == 0) 
        {
            for (int j = 0; j < global_config.sections[i].key_count; j++) 
            {
                if (strcmp(global_config.sections[i].keys[j].key, key) == 0) 
                {
                    return global_config.sections[i].keys[j].value;
                }
            }
        }
    }
    return NULL;  
}

int save_to_config(const char * section, const char * key, const char * value)
{
    int sec_idx = -1;
    for (int i = 0; i < global_config.section_count; i++) 
    {
        if (strcmp(global_config.sections[i].section, section) == 0) 
        {
            sec_idx = i;
            break;
        }
    }
    if (sec_idx == -1) 
    {
        if (global_config.section_count >= MAX_SECTION_COUNT) 
        {
            log_e("Section over max count");
            return -1;  // 返回错误：section 数量超出限制
        }
        sec_idx = global_config.section_count++;
        strncpy(global_config.sections[sec_idx].section, section, MAX_NAME_LENGTH);
        global_config.sections[sec_idx].key_count = 0;
    }

    // 保存 key-value 到对应 section
    Section *sec = &global_config.sections[sec_idx];
    for (int i = 0; i < sec->key_count; i++) 
    {
        // 如果找到相同的 key，更新其值
        if (strcmp(sec->keys[i].key, key) == 0) 
        {
            strncpy(sec->keys[i].value, value, MAX_VALUE_LENGTH);
            return 0;  // 返回成功：更新已存在的键值
        }
    }

    // 如果没有找到相同的 key，添加新的 key-value
    if (sec->key_count >= MAX_KEY_COUNT) {
        log_e("Key over max count");
        return -2;  // 返回错误：key 数量超出限制
    }
    strncpy(sec->keys[sec->key_count].key, key, MAX_NAME_LENGTH);
    strncpy(sec->keys[sec->key_count].value, value, MAX_VALUE_LENGTH);
    sec->key_count++;
    return 0;  // 返回成功：添加新的键值对
}

int write_ini()
{
    FILE *file = fopen(CONFIG_FILE, "w");
    if (!file) {
        log_e("Can't open file: %s", CONFIG_FILE);
        return -1;
    }

    for (int i = 0; i < global_config.section_count; i++) 
    {
        fprintf(file, "[%s]\n", global_config.sections[i].section);
        for (int j = 0; j < global_config.sections[i].key_count; j++) 
        {
            fprintf(file, "%s = %s\n",
                    global_config.sections[i].keys[j].key,
                    global_config.sections[i].keys[j].value);
        }
        fprintf(file, "\n");
    }

    fclose(file);
}


