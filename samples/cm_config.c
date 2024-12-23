#include <stdlib.h>
#include <unistd.h>
#include "cm_conf.h"
#include "cm_config.h"
#include "cm_common.h"

#define CM_CONFIG_FILE "/system/etc/cm_config.ini"

static CMConfig cm_config_local[32];

static DoorConfigArgs s_door_config = { 4.0f, 4.0f, 36.0f, 300 };
// static int wind_debug_param = 0;
int cm_config_load()
{
    int ret = cm_config_read(CM_CONFIG_FILE, cm_config_local, 20);
    if (ret > 0)
    {
        for (int i = 0; i < ret; i++)
        {
            LOGD("key [%s] value [%s]", cm_config_local[i].key, cm_config_local[i].value);
        }
        cm_config_get_angle_open(&s_door_config.angle_open);
        cm_config_get_angle_close(&s_door_config.angle_close);
        cm_config_get_angle_max(&s_door_config.angle_max);
        s_door_config.door_open_timeout = cm_config_get_door_open_timeout();
        LOGD("cm_config_load: angle_open = %f, angle_close = %f, angle_max = %f, door_open_timeout = %d",
             s_door_config.angle_open, s_door_config.angle_close,
             s_door_config.angle_max, s_door_config.door_open_timeout);
    }
    else
    {
        LOGD("Error: cm_config_load failed. Use default");
    }
    return 0;
}

int cm_config_save()
{
    return cm_config_write(CM_CONFIG_FILE, cm_config_local, 20);
}

int cm_config_get_angle_open(float *angle)
{
    char *v = cm_config_value(cm_config_local, 20, "angle_open");
    if (v != NULL)
    {
        *angle = atof(v);
    }
    else
    {
        *angle = 4.0f;
    }
    return 0;
}

int cm_config_get_angle_close(float *angle)
{
    char *v = cm_config_value(cm_config_local, 20, "angle_close");
    if (v != NULL)
    {
        *angle = atof(v);
    }
    else
    {
        *angle = 4.0f;
    }
    return 0;
}

int cm_config_get_angle_max(float *angle)
{
    char *v = cm_config_value(cm_config_local, 20, "angle_max");
    if (v != NULL)
    {
        *angle = atof(v);
    }
    else
    {
        *angle = 36.0f;
    }
    return 0;
}

int cm_config_set_angle_open(float angle)
{
    if (angle < 0 || angle > 180.0f)
    {
        LOGD("invalid angle_open value [%f] (0-180.0)", angle);
        return -1;
    }
    LOGD("set angle open [%f] success", angle);
    s_door_config.angle_open = angle;
    char angle_value[10];
    snprintf(angle_value, 10, "%f", angle);
    cm_config_set_value(&cm_config_local[0], 20, "angle_open", angle_value);
    cm_config_save();
    return 0;
}

int cm_config_set_angle_close(float angle)
{
    if (angle < 0 || angle > 180.0f)
    {
        LOGD("invalid angle_close value [%f] (0-180.0)", angle);
        return -1;
    }
    LOGD("set angle close [%f] success", angle);
    s_door_config.angle_close = angle;
    char angle_value[10];
    snprintf(angle_value, 10, "%f", angle);
    cm_config_set_value(&cm_config_local[0], 20, "angle_close", angle_value);
    cm_config_save();
    return 0;
}

int cm_config_set_angle_max(float angle)
{
    if (angle < 0 || angle > 180.0f)
    {
        LOGD("invalid angle_max value [%f] (0-180.0)", angle);
        return -1;
    }
    LOGD("set angle max [%f] success", angle);
    s_door_config.angle_max = angle;
    char angle_value[10];
    snprintf(angle_value, 10, "%f", angle);
    cm_config_set_value(&cm_config_local[0], 20, "angle_max", angle_value);
    cm_config_save();
    return 0;
}

static DoorInfoItem s_door_item[DOOR_INFO_ITEM_MAX];// 只存储一定数据 

int cm_door_info_item_add(DoorInfoItem *item)
{
    if (!item)
    {
        return -2;
    }
    if (item->array_index >= DOOR_INFO_ITEM_MAX)
    {
        LOGD("array_index out of range [%d]\n", item->array_index);
        return -1;
    }
    s_door_item[item->array_index] = *item;
    return 0;
}

DoorInfoItem* cm_door_info_item_get(int index)
{
    if (index >= DOOR_INFO_ITEM_MAX)
    {
        LOGD("array_index out of range [%d]\n", index);
        return NULL;
    }
    return &s_door_item[index];
}

int cm_config_get_door_open_timeout()
{
    char *v = cm_config_value(cm_config_local, 20, "door_open_timeout");
    if (v != NULL)
    {
        int tm = strtol(v, NULL, 10);
        if (tm >= 5 && tm <= 1000000)
        {
            return tm;
        }
    }
    return 300;
}

int cm_config_set_door_open_timeout(int timeout)
{
    if (timeout < 5 || timeout > 1000000)
    {
        LOGD("invalid timeout value [%d] (5-1000000", timeout);
        return -1;
    }
    s_door_config.door_open_timeout = timeout;
    char value[10];
    snprintf(value, 10, "%d", timeout);
    cm_config_set_value(&cm_config_local[0], 20, "door_open_timeout", value);
    cm_config_save();
    return 0;
}

int cm_is_debug()
{
    return (access("/system/etc/debug", F_OK) == 0);
}
int cm_is_debug2()
{
    return (access("/system/etc/debug1", F_OK) == 0);
}
DoorConfigArgs *cm_get_door_config()
{
    return &s_door_config;
}
