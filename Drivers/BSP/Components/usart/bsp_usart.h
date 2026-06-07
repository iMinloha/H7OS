#ifndef BSP_USART_H
#define BSP_USART_H

#include "bsp_file_ops.h"

extern bsp_file_ops_t usart1_fops;

/** Call from device_init() to register USART1 under /dev */
void usart1_device_init(void);

#endif
