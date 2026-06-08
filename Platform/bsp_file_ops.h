#ifndef BSP_FILE_OPS_H
#define BSP_FILE_OPS_H

#include <stdint.h>

/* Linux-style 精简 file_operations */
typedef struct bsp_file_ops {
    int  (*open)  (void *dev);
    int  (*close) (void *dev);
    int  (*read)  (void *dev, uint8_t *buf, uint32_t len);
    int  (*write) (void *dev, const uint8_t *buf, uint32_t len);
} bsp_file_ops_t;

#endif
