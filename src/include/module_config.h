#ifndef __MODULE_CONFIG_H__
#define __MODULE_CONFIG_H__

#include<stdint.h>

#define MAX_CONFIG_FILE_SIZE              4096
#define MAX_KEY_LENGTH                    16
#define MAX_VALUE_LENGTH                  96

#define CONFIG_OPS_SUCCESS                0
#define CONFIG_OPS_FAIL                   -1
#define CONFIG_RECORD_NOT_EXSIT           -2

int module_config_init(char *cfg_file_path);
int module_config_deinit(void);

/* add new record */
int module_config_add_string(const char *key, char *value);
int module_config_add_int(const char *key, int value);
int module_config_add_float(const char *key, float value);
int module_config_add_hex(const char *key, int value);

/**
 * update record
 * if record not exist and create == 1,
 * it will add the new record to config file.
 **/
int module_config_update_string(const char *key, char *value, int create);
int module_config_update_int(const char *key, int value, int create);
int module_config_update_float(const char *key, float value, int create);
int module_config_update_hex(const char *key, int value, int create);

/* get existed record */
int module_config_get_string(const char *key, char *value);
int module_config_get_int(const char *key, int *value);
int module_config_get_float(const char *key, float *value);
int module_config_get_hex(const char *key, int *value);
int module_config_get_uhex(const char *key, uint8_t *value,int num);

/* delete record */
int module_config_delete(const char *key);

int module_config_get_mult_fps(char *key, unsigned int *fps_list);
int module_config_get_mult_maxpacksize(char *key, unsigned int *mps_list);
int module_config_get_mult_frames(int frames_num, unsigned int *frame_list);

#endif /* __MODULE_CONFIG_H__ */
