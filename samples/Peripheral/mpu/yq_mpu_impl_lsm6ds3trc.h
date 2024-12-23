/**
 * Create: 20180515
 * Author: jmdvirus
 */

#ifndef __CM_COMMON_H
#define __CM_COMMON_H

#define API_PREFIX

#ifdef __cplusplus
extern "C"
{
#endif

#define LOGD(fmt, ...)                          \
    do                                          \
    {                                           \
        printf("[%s:%d] ", __func__, __LINE__); \
        printf(fmt, ##__VA_ARGS__);             \
    } while (0)


#ifdef __cplusplus
}
#endif

#endif
