
#ifndef __CM_CONFIG_H__
#define __CM_CONFIG_H__

#define DOOR_INFO_ITEM_MAX 10
// 当次开门缓存下来的信息
typedef struct {
    float image_angle;
    char image_path[256];
    unsigned long long open_count;
    int array_index;
} DoorInfoItem;

typedef struct {
    float angle_open;
    float angle_close;
    float angle_max;
    int door_open_timeout;
} DoorConfigArgs;

// 在此文件中管理开门后的信息，统一在这里添加和访问
int cm_door_info_item_add(DoorInfoItem *item);
DoorInfoItem* cm_door_info_item_get(int index);

int cm_config_load();
int cm_config_save();

int cm_config_get_angle_open(float *angle);
int cm_config_get_angle_close(float *angle);

int cm_config_get_angle_max(float *angle);

int cm_config_get_door_open_timeout();

int cm_config_set_angle_open(float angle);
int cm_config_set_angle_close(float angle);

int cm_config_set_angle_max(float angle);

int cm_config_set_door_open_timeout(int timeout);

int cm_is_debug();

DoorConfigArgs *cm_get_door_config();

#endif