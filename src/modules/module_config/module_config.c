#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <pthread.h>
#include <sys/time.h>
#include <errno.h>
#include <ctype.h>

#include <module_config.h>

#define MODULE_TAG     "CONFIG"

#define MAX_CONFIG_FILE_LINE_LENGTH       128

typedef struct _config_context {
	FILE *fp;
	int fd;
	char *fbuf;
	pthread_mutex_t mutex;
	fpos_t line_pos;
} config_context_t;

static config_context_t cfg_context;

static inline void strim(char *str)
{
	char *p = str;

	while (*p != '\0') {
		if ((*p == '\t') || (*p == '\n') || (*p == '\v') ||
		    (*p == '\f') || (*p == '\r')) {
			*p = '\0';
			break;
		}
		p++;
	}
}

/**
 * file operation
 **/
static inline void set_pos_to_begin(void)
{
	fseek(cfg_context.fp, 0L, SEEK_SET);
}

static inline void set_pos_to(fpos_t pos)
{
	/*fseek(cfg_context.fp, pos, SEEK_SET);*/
	fsetpos(cfg_context.fp, &pos);
}

static inline void set_pos_to_end(void)
{
	fseek(cfg_context.fp, 0L, SEEK_END);
}

static int get_line_by_key(const char *key, char *line)
{
	int ret = 0;
	char *line_ptr;
	int key_len = strlen(key);

	pthread_mutex_lock(&cfg_context.mutex);

	set_pos_to_begin();
	do {
		line_ptr = fgets(cfg_context.fbuf, MAX_CONFIG_FILE_LINE_LENGTH, cfg_context.fp);

		if (line_ptr && (!strncmp(line_ptr, key, key_len))) {
			/*if (line_ptr[key_len] == ':') {*/
			if (strchr(line_ptr, ':')) {
				strcpy(line, line_ptr);
				ret = fgetpos(cfg_context.fp, &cfg_context.line_pos);
				if(ret < 0) {
					printf("ERROR[%s]: fgetpos failed!\n", MODULE_TAG);
					return ret;
				}
				break;
			}
		}

	} while (line_ptr);

	if (!line_ptr) {
		ret = CONFIG_RECORD_NOT_EXSIT;
	}

	pthread_mutex_unlock(&cfg_context.mutex);

	return ret;
}

static int add_line_to_tail(const char *line)
{
	int ret = 0;

	pthread_mutex_lock(&cfg_context.mutex);

	set_pos_to_end();
	ret = fputs(line, cfg_context.fp);
	if (ret < 0) {
		printf("ERROR(%s): update config file failed! %s\n", MODULE_TAG, strerror(errno));
		ret = CONFIG_OPS_FAIL;
	} else {
		fflush(cfg_context.fp);
		fsync(cfg_context.fd);
		ret = 0;
	}

	pthread_mutex_unlock(&cfg_context.mutex);

	return ret;
}

static int delete_line_by_key(const char *key)
{
	int ret = 0;
	char *line_ptr;
	int key_len = strlen(key);
	int pos = 0, rest_len, wlen;

	pthread_mutex_lock(&cfg_context.mutex);

	set_pos_to_begin();
	do {
		line_ptr = fgets(cfg_context.fbuf + pos, MAX_CONFIG_FILE_LINE_LENGTH, cfg_context.fp);

		if (line_ptr) {
			if ((!strncmp(line_ptr, key, key_len)) && (line_ptr[key_len] == ':')) {
				break;
			}
			pos += strlen(line_ptr);
		}
	} while (line_ptr);

	if (line_ptr) {
		rest_len = fread(cfg_context.fbuf + pos, 1, MAX_CONFIG_FILE_SIZE, cfg_context.fp);
		if ((rest_len < 0) || (rest_len > MAX_CONFIG_FILE_SIZE)) {
			printf("ERROR(%s): read config file failed! %s\n", MODULE_TAG, strerror(errno));
			ret = CONFIG_OPS_FAIL;
			goto err;
		}

		ret = ftruncate(cfg_context.fd, 0);
		if (ret) {
			printf("ERROR(%s): cleanup config file failed! %s\n", MODULE_TAG, strerror(errno));
			ret = CONFIG_OPS_FAIL;
			goto err;
		}

		set_pos_to_begin();
		wlen = fwrite(cfg_context.fbuf, 1, pos + rest_len, cfg_context.fp);
		if (wlen != (pos + rest_len)) {
			printf("ERROR(%s): write config file failed! %s\n", MODULE_TAG, strerror(errno));
			ret = CONFIG_OPS_FAIL;
			goto err;
		}

		fflush(cfg_context.fp);
		fsync(cfg_context.fd);
	} else {
		ret = CONFIG_RECORD_NOT_EXSIT;
	}

err:
	pthread_mutex_unlock(&cfg_context.mutex);

	return ret;
}

static int replace_line_by_key(const char *key, const char *line)
{
	int ret = 0;
	char *line_ptr;
	int key_len = strlen(key);
	int pos = 0, rest_len, wlen;

	pthread_mutex_lock(&cfg_context.mutex);

	set_pos_to_begin();
	do {
		line_ptr = fgets(cfg_context.fbuf + pos, MAX_CONFIG_FILE_LINE_LENGTH, cfg_context.fp);
		if (line_ptr) {
			if ((!strncmp(line_ptr, key, key_len)) && (line_ptr[key_len] == ':')) {
				break;
			}
			pos += strlen(line_ptr);
		}
	} while (line_ptr);

	if (line_ptr) {
		memcpy(cfg_context.fbuf + pos, line, strlen(line));
		pos += strlen(line);

		rest_len = fread(cfg_context.fbuf + pos, 1, MAX_CONFIG_FILE_SIZE, cfg_context.fp);
		if ((rest_len < 0) || (rest_len > MAX_CONFIG_FILE_SIZE)) {
			printf("ERROR(%s): read config file failed! %s\n", MODULE_TAG, strerror(errno));
			ret = CONFIG_OPS_FAIL;
			goto err;
		}

		ret = ftruncate(cfg_context.fd, 0);
		if (ret) {
			printf("ERROR(%s): cleanup config file failed! %s\n", MODULE_TAG, strerror(errno));
			ret = CONFIG_OPS_FAIL;
			goto err;
		}

		set_pos_to_begin();
		wlen = fwrite(cfg_context.fbuf, 1, pos + rest_len, cfg_context.fp);
		if (wlen != (pos + rest_len)) {
			printf("ERROR(%s): write config file failed! %s\n", MODULE_TAG, strerror(errno));
			ret = CONFIG_OPS_FAIL;
			goto err;
		}

		fflush(cfg_context.fp);
		fsync(cfg_context.fd);
	} else {
		ret = CONFIG_RECORD_NOT_EXSIT;
	}

err:
	pthread_mutex_unlock(&cfg_context.mutex);

	return ret;
}

int module_config_add_string(const char *key, char *value)
{
	int ret = 0;
	char line_buf[MAX_CONFIG_FILE_LINE_LENGTH];

	if (value)
		strim(value);
	sprintf(line_buf, "%s:%s\n", key, value);
	ret = add_line_to_tail(line_buf);

	return ret;
}

int module_config_add_int(const char *key, int value)
{
	int ret = 0;
	char line_buf[MAX_CONFIG_FILE_LINE_LENGTH];

	sprintf(line_buf, "%s:%d\n", key, value);
	ret = add_line_to_tail(line_buf);

	return ret;
}

int module_config_add_hex(const char *key, int value)
{
	int ret = 0;
	char line_buf[MAX_CONFIG_FILE_LINE_LENGTH];

	sprintf(line_buf, "%s:0x%x\n", key, value);
	ret = add_line_to_tail(line_buf);

	return ret;
}

int module_config_add_float(const char *key, float value)
{
	int ret = 0;
	char line_buf[MAX_CONFIG_FILE_LINE_LENGTH];

	sprintf(line_buf, "%s:%f\n", key, value);
	ret = add_line_to_tail(line_buf);

	return ret;
}

int module_config_update_string(const char *key, char *value, int create)
{
	int ret = 0;
	char line_buf[MAX_CONFIG_FILE_LINE_LENGTH];

	if (value)
		strim(value);
	sprintf(line_buf, "%s:%s\n", key, value);
	ret = replace_line_by_key(key, line_buf);
	if ((ret == CONFIG_RECORD_NOT_EXSIT) && (create == 1)) {
		ret = module_config_add_string(key, value);
	}

	return ret;
}

int module_config_update_int(const char *key, int value, int create)
{
	int ret = 0;
	char line_buf[MAX_CONFIG_FILE_LINE_LENGTH];

	sprintf(line_buf, "%s:%d\n", key, value);
	ret = replace_line_by_key(key, line_buf);
	if ((ret == CONFIG_RECORD_NOT_EXSIT) && (create == 1)) {
		ret = module_config_add_int(key, value);
	}

	return ret;
}

int module_config_update_hex(const char *key, int value, int create)
{
	int ret = 0;
	char line_buf[MAX_CONFIG_FILE_LINE_LENGTH];

	sprintf(line_buf, "%s:0x%x\n", key, value);
	ret = replace_line_by_key(key, line_buf);
	if ((ret == CONFIG_RECORD_NOT_EXSIT) && (create == 1)) {
		ret = module_config_add_hex(key, value);
	}

	return ret;
}

int module_config_update_float(const char *key, float value, int create)
{
	int ret = 0;
	char line_buf[MAX_CONFIG_FILE_LINE_LENGTH];

	sprintf(line_buf, "%s:%f\n", key, value);
	ret = replace_line_by_key(key, line_buf);
	if ((ret == CONFIG_RECORD_NOT_EXSIT) && (create == 1)) {
		ret = module_config_add_float(key, value);
	}

	return ret;
}

int module_config_get_string(const char *key, char *value)
{
	int ret = 0;
	char *ptr = NULL;
	char line_buf[MAX_CONFIG_FILE_LINE_LENGTH];

	ret = get_line_by_key(key, line_buf);
	if (!ret) {
		if (strtok(line_buf, ":")) {
			ptr = strtok(NULL, ":");
		}

		if (ptr) {
			strim(ptr);
			strcpy(value, ptr);
		} else
			return CONFIG_OPS_FAIL;
	}

	return ret;
}

int module_config_get_int(const char *key, int *value)
{
	int ret = 0;
	char *ptr = NULL;
	char line_buf[MAX_CONFIG_FILE_LINE_LENGTH];

	ret = get_line_by_key(key, line_buf);
	if (!ret) {
		if (strtok(line_buf, ":")) {
			ptr = strtok(NULL, ":");
		}

		if (ptr) {
			strim(ptr);
			*value = atoi(ptr);
		} else
			return CONFIG_OPS_FAIL;
	}

	return ret;
}

int module_config_get_uhex(const char *key, uint8_t *value,int num)
{
	int ret = 0;
	int i = 0;
	char *ptr = NULL;
	char line_buf[MAX_CONFIG_FILE_LINE_LENGTH];

	ret = get_line_by_key(key, line_buf);
	if (!ret) {
		if (strtok(line_buf, ":")) {
			ptr = strtok(NULL, ":");
			ptr = strtok(ptr,",");
			memset(value,0,num);
		}

		if (ptr) {
			while((ptr != NULL) && (i != num))
			{
				strim(ptr);
				//printf("INFO[%s]ptr:%s num:%d \n",__func__,ptr,num);
				value[i++] = (uint8_t)strtol(ptr, NULL, 16); // 0x32 -> 50
				ptr = strtok(NULL,",");
				//printf("INFO[%s]:%hhu %d\n",__func__,value[i-1],i);
			}

		} else
			return CONFIG_OPS_FAIL;
	}

	return ret;
}

int module_config_get_hex(const char *key, int *value)
{
	int ret = 0;
	char *ptr = NULL;
	char line_buf[MAX_CONFIG_FILE_LINE_LENGTH];

	ret = get_line_by_key(key, line_buf);
	if (!ret) {
		if (strtok(line_buf, ":")) {
			ptr = strtok(NULL, ":");
		}

		if (ptr) {
			strim(ptr);
			*value = (int)strtol(ptr, NULL, 16); // 0x32 -> 50
		} else
			return CONFIG_OPS_FAIL;
	}

	return ret;
}

int module_config_get_float(const char *key, float *value)
{
	int ret = 0;
	char *ptr = NULL;
	char line_buf[MAX_CONFIG_FILE_LINE_LENGTH];

	ret = get_line_by_key(key, line_buf);
	if (!ret) {
		if (strtok(line_buf, ":")) {
			ptr = strtok(NULL, ":");
		}

		if (ptr) {
			strim(ptr);
			*value = atof(ptr);
		} else
			return CONFIG_OPS_FAIL;
	}

	return ret;
}

int module_config_delete(const char *key)
{
	int ret = 0;

	ret = delete_line_by_key(key);

	return ret;
}

int module_config_get_mult_maxpacksize(char *key, unsigned int* mps_list)
{
	int ret = 0;
	char *ptr = NULL;
	char line_buf[MAX_CONFIG_FILE_LINE_LENGTH];
	int mps_num = 0;

	if (!mps_list){
		return -1;
	}

	ret = get_line_by_key(key, line_buf);
	if (!ret) {
		if (strtok(line_buf, ":")) {
			ptr = strtok(NULL, ":");
		}

		if (ptr) {
			strim(ptr);
			ptr = strtok(ptr, "/");
			while(ptr != NULL) {
				mps_list[mps_num] = atoi(ptr);
				ptr = strtok(NULL, "/");
				mps_num++;
			}
		} else
			return CONFIG_OPS_FAIL;
	} else
		return -1;


	return 0;
}

int module_config_get_mult_fps(char *key, unsigned int* fps_list)
{
	int ret = 0;
	char *ptr = NULL;
	char line_buf[MAX_CONFIG_FILE_LINE_LENGTH];
	int fps_num = 0;

	if (!fps_list){
		return -1;
	}

	ret = get_line_by_key(key, line_buf);
	if (!ret) {
		if (strtok(line_buf, ":")) {
			ptr = strtok(NULL, ":");
		}

		if (ptr) {
			strim(ptr);
			ptr = strtok(ptr, "/");
			while(ptr != NULL) {
				fps_list[fps_num] = 10000000 / atoi(ptr);
				ptr = strtok(NULL, "/");
				fps_num++;
			}
		} else
			return CONFIG_OPS_FAIL;
	}

	return 0;
}

int module_config_get_mult_frames(int frames_num, unsigned int* frame_list)
{
	int i = 0;
	unsigned int width, height;

	if (!frame_list){
		return -1;
	}

	pthread_mutex_lock(&cfg_context.mutex);
	set_pos_to(cfg_context.line_pos);
	while ((i++ < frames_num)) {
		if (fscanf(cfg_context.fp, "{%d, %d}", &width, &height) < 0)
			break;
		if (width > 0 && height > 0 && (width % 16 == 0)) {
			/* 不能使用以下方式赋值，程序会出现do page fault 错误
			 *frame_list[i++] = width;
			 *frame_list[i++] = height;
			 */
			*frame_list++ = width;
			*frame_list++ = height;
			/*printf("width %d, height %d\n", width, height);*/
		} else if (width > 0 && height > 0 && (height % 16 == 0)) {
			if((width == (height * 9 / 16))     || \
				(width == (height * 3 / 4)) || \
				(width == (height * 9 / 20))) {
				*frame_list++ = width;
				*frame_list++ = height;
			}
		}

		fseek(cfg_context.fp, 1, SEEK_CUR);
	}

	pthread_mutex_unlock(&cfg_context.mutex);
	return 0;
}

int module_config_init(char *cfg_file_path)
{
	/* if file not exsit, create one */
	if (access(cfg_file_path, F_OK)) {
		cfg_context.fp = fopen(cfg_file_path, "w+");
	} else {
		cfg_context.fp = fopen(cfg_file_path, "r+");
	}
	if (!cfg_context.fp) {
		printf("ERROR(%s): module_config_init open file [%s] failed! %s\n",
		       MODULE_TAG, cfg_file_path, strerror(errno));
		return -1;
	}

	cfg_context.fd = fileno(cfg_context.fp);

	cfg_context.fbuf = (char *)malloc(MAX_CONFIG_FILE_SIZE);
	if (!cfg_context.fbuf) {
		printf("ERROR(%s): module_config_init alloc memory failed! %s\n",
		       MODULE_TAG, strerror(errno));
		fclose(cfg_context.fp);
		return -1;
	}

	pthread_mutex_init(&cfg_context.mutex, NULL);

	return 0;
}

int module_config_deinit(void)
{
	if (cfg_context.fbuf) {
		free(cfg_context.fbuf);
		cfg_context.fbuf = NULL;
	}

	if (cfg_context.fp) {
		fclose(cfg_context.fp);
		cfg_context.fp = NULL;
		cfg_context.fd = 0;
	}
	printf("%s...OK\n",__func__);
	return 0;
}
