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

#define CM_CHECK_LOCK(name, mutex)                                             \
    {                                                                          \
        int          count                  = 0;                               \
        int          mutex_line             = __LINE__;                        \
        static int   last_mutex_line_##name = 0;                               \
        static char *last_mutex_func_##name = (char *)"null";                  \
        while (pthread_mutex_trylock(mutex) != 0)                              \
        {                                                                      \
            count++;                                                           \
            if (count > 400)                                                   \
            {                                                                  \
                if (count % 100 == 0)                                          \
                    printf("[%s:%d] deadlock last lock at %s:%d\n", __func__,  \
                           __LINE__, last_mutex_func_##name,                   \
                           last_mutex_line_##name);                            \
            }                                                                  \
            usleep(10000);                                                     \
        }                                                                      \
        pthread_mutex_lock(mutex);                                             \
        last_mutex_line_##name = mutex_line;                                   \
        last_mutex_func_##name = (char *)__func__;                             \
    }

#define LOGD(fmt, ...)                                                         \
    do                                                                         \
    {                                                                          \
        printf("[%s:%d] ", __func__, __LINE__);                                \
        printf(fmt, ##__VA_ARGS__);                                            \
        printf("\n");\
    } while (0)

#ifdef GCC_HIGH
#define CM_CPP_OVERRIDE override
#else
#define CM_CPP_OVERRIDE
#endif

    enum
    {
        CM_E_OK = 0,
        CM_E_DONE = 1,
        CM_E_ARGS = -10,
        CM_E_NO_MEM = -11,
    };

#ifdef __cplusplus
}
#endif

#endif
