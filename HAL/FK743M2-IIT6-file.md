# FK743M2-IIT6 板级目录快照

> 记录时间: 2026-06-09 (CubeMX 重新生成前)
> 二进制: FLASH=159936 B, RAM_D1=83264 B
> .bin hash: bdf1bb12865c7e511ef831655c7bdff0

## 文件清单

### BSP/ (19 文件 — 手写，CubeMX 不生成，必须保留)
```
BSP/adc/bsp_adc.c              BSP/i2c/bsp_i2c.c
BSP/adc/bsp_adc.h              BSP/i2c/bsp_i2c.h
BSP/bsp_devices.c              BSP/lcd/lcd_define.c
BSP/cpu/bsp_cpu.c              BSP/lcd/lcd_define.h
BSP/cpu/bsp_cpu.h              BSP/pwm/bsp_pwm.c
BSP/gpio/bsp_gpio.c            BSP/pwm/bsp_pwm.h
BSP/gpio/bsp_gpio.h            BSP/touch/torch_iic.c
BSP/hal_init.c                 BSP/touch/torch_iic.h
BSP/hal_init.h                 BSP/usart/bsp_usart.c
                               BSP/usart/bsp_usart.h
```
> BSP/sd/ 为预存空目录，非本次产生

### Drivers/ (CubeMX 通用库)
- **CMSIS/** — 30 头文件 + 2 LICENSE.txt
- **STM32H7xx_HAL_Driver/** — 约 120 .h + 50 .c + LICENSE.txt

### Inc/ (19 文件 — CubeMX 外设头文件)
```
adc.h  dma.h  dma2d.h  fmc.h  gpio.h  i2c.h  jpeg.h  jpeg_utils_conf.h
ltdc.h  main.h  mdma.h  quadspi.h  rng.h  rtc.h  sdmmc.h
stm32h7xx_hal_conf.h  stm32h7xx_it.h  tim.h  usart.h
```

### Src/ (19 文件 — CubeMX 外设源文件)
```
adc.c  dma.c  dma2d.c  fmc.c  gpio.c  i2c.c  jpeg.c
ltdc.c  mdma.c  quadspi.c  rng.c  rtc.c  sdmmc.c
stm32h7xx_hal_msp.c  stm32h7xx_hal_timebase_tim.c
stm32h7xx_it.c  system_stm32h7xx.c  tim.c  usart.c
```
> 无 main.c (CMake 排除，真正入口在 Core/Src/main.c)

### 根目录文件 (4 文件)
```
FK743M2-IIT6.ioc               # CubeMX 工程
STM32H743IITX_FLASH.ld         # FLASH 链接脚本
STM32H743IITX_RAM.ld           # RAM 链接脚本
Startup/startup_stm32h743iitx.s # 启动汇编
```

## CubeMX 覆盖规则
| 目录 | 覆盖 | 备注 |
|------|:--:|------|
| BSP/ | ❌ | 手写，不生成 |
| Drivers/ | ✅ | 标准库 |
| Inc/ Src/ Startup/ | ✅ | 外设代码 |
| *.ioc *.ld | ✅ | 工程文件 |

## CMake 关键配置
```cmake
# HAL_SOURCES 排除 Drivers/ (由 DRIVER_SOURCES 单独管理)
list(FILTER HAL_SOURCES EXCLUDE REGEX ".*/Drivers/.*")

# DRIVER_SOURCES 显式管理，不混入其他 glob
file(GLOB_RECURSE DRIVER_SOURCES
    "HAL/${BOARD}/Drivers/STM32H7xx_HAL_Driver/*.*"
    "HAL/${BOARD}/Drivers/CMSIS/*.*"
    "Drivers/Kernel/*.*"
)
```
