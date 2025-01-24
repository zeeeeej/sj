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
/*陀螺仪代码修改
1、malloc free
2、shell command
3、dev iic 
4、consr device
5、knlog
6、gyro*/
    typedef enum
    {
        YQ_ERROR_SUCCESS = 0,   // 一般指动作结束， 没有产生错误
        YQ_ERROR_DONE = 1,      // 一般指读取操作成功，有数据
        YQ_ERROR_NO_MEM = -100, // -99 -98
        YQ_ERROR_ARGS,
        YQ_ERROR_NO_PERMISSION,
        YQ_ERROR_NOT_EXIST,
        YQ_ERROR_HAS_EXIST,
        YQ_ERROR_FILE_OPEN,
        YQ_ERROR_FILE_CLOSE,
        YQ_ERROR_FILE_READ,
        YQ_ERROR_FILE_WRITE,
        YQ_ERROR_NOT_INITED,
        YQ_ERROR_HAS_INITED,
        YQ_ERROR_NOT_STARTED,
        YQ_ERROR_HAS_STARTED,

        YQ_ERROR_CUSTOM_NEXT = -1000,
    } YQError;


#ifdef __cplusplus
}
#endif

#endif
