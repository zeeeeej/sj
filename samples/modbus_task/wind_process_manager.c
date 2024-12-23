//
// Created by DELL on 24-9-20.
//

#include <stdio.h>

#include "wind_process_manager.h"
#include "cm_common.h"

static WindProcessManager s_process_manager;

int wind_process_manager_status_changed(int st)
{
    LOGD("status changed from [%d] to [%d]\n", s_process_manager.status, st);
    s_process_manager.status = st;
    return 0;
}

int wind_process_manager_connect_status_changed(int st)
{
    LOGD("connect status changed from [%d] to [%d]\n", s_process_manager.connect_status, st);
    s_process_manager.connect_status = st;
    return 0;
}

int wind_process_manager_get_status(void)
{
    return s_process_manager.status;
}

int wind_process_manager_get_connect_status(void)
{
    return s_process_manager.connect_status;
}
