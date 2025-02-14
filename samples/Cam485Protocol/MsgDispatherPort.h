#ifndef MSG_DISPATHER_PORT_H__
#define MSG_DISPATHER_PORT_H__


#include "stdint.h"
void print_485_cir_buf_contents();
int get_RS485uart_fd(void);
void set_RS485uart_fd(uint32_t uartFd);

#endif