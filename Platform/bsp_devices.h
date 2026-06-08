#ifndef BSP_DEVICES_H
#define BSP_DEVICES_H

#include <stdint.h>

void devices_init(void);
void board_periph_init(void);
uint64_t board_sd_get_capacity(void);
void board_rtc_set_dfu_flag(void);

#endif
