#ifndef _PARSE_INI_H_
#define _PARSE_INI_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define CONFIG_FILE "/system/etc/cm_config.ini"
#define MAX_SECTION_COUNT 10
#define MAX_KEY_COUNT 50
#define MAX_NAME_LENGTH 128
#define MAX_VALUE_LENGTH 128


typedef void (*IniCallback)(const char *section, const char *key, const char *value);


typedef struct {
    char key[MAX_NAME_LENGTH];
    char value[MAX_VALUE_LENGTH];
} KeyValue;

typedef struct {
    char section[MAX_NAME_LENGTH];
    KeyValue keys[MAX_KEY_COUNT];
    int key_count;
} Section;

typedef struct {
    Section sections[MAX_SECTION_COUNT];
    int section_count;
} Config;

// 全局配置
extern Config global_config;

char* trim(char* str);
int parse_ini();
const char *get_config_value(const char *section, const char *key);
int save_to_config(const char * section, const char * key, const char * value);
int write_ini();
#endif