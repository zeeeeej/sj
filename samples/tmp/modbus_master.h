#ifndef __MODBUS_MASTER_H__
#define __MODBUS_MASTER_H__

typedef struct {
    int (*init)(void);
} modbus_master_interface;


#endif  