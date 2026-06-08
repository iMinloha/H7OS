# 添加新板子指南

H7OS 通过 HAL → Platform → Software 三层架构支持多板型。切换板子只需修改 CMakeLists.txt 中的 `BOARD` 变量。

## 目录结构

```
HAL/_template/                    # 新板子模板 (复制此目录即可开始)
HAL/FK743M2-IIT6/                 # 参考实现: FK743M2-IIT6 开发板
Platform/Board/
  board_select.h                  # 板级选择器 (在此注册新板子)
  board_template.h                # 板级配置模板 (复制此文件)
  board_fk743m2.h                 # 参考配置: FK743M2-IIT6
```

## 添加步骤

### 1. 创建 HAL 板子目录

```bash
cp -r HAL/_template HAL/MyBoard
```

### 2. 用 CubeMX 生成代码

1. 打开 STM32CubeMX
2. 在 `HAL/MyBoard/` 中创建 `.ioc` 工程，命名为 `MyBoard.ioc`
3. 配置引脚、时钟树、外设
4. **Project Manager** → **Code Generator** → 取消勾选 "Generate main()" 避免生成 main.c
5. 生成代码到 `HAL/MyBoard/`

生成的目录:
```
HAL/MyBoard/
├── Inc/             # 外设头文件 (gpio.h, usart.h, ...)
├── Src/             # 外设源文件 (gpio.c, usart.c, ...)
├── Startup/         # 启动文件
├── MyBoard.ioc      # CubeMX 工程
├── *.ld             # 链接脚本
├── BSP/             # 板级 BSP (从 _template 复制)
│   ├── hal_init.c   #   HAL_BoardInit()
│   ├── bsp_devices.c #   devices_init() + board_periph_init()
│   └── gpio/ usart/ #   外设 BSP 驱动
└── hal_init.h       # HAL_BoardInit() 声明
```

### 3. 填充 hal_init.c

从 CubeMX 生成的 `Src/main.c` 中复制三个函数到 `BSP/hal_init.c`:

- `SystemClock_Config()` — 完整复制
- `MPU_Config()` — 完整复制 (如果有 SDRAM)
- `PeriphCommonClock_Config()` — 完整复制

并在 `HAL_BoardInit()` 中添加所有 `MX_xxx_Init()` 调用 (从 main.c 的 `main()` 函数中按顺序复制)。

### 4. 创建板级配置

复制 `Platform/Board/board_template.h` → `Platform/Board/board_myboard.h`:

```c
#ifdef BOARD_MYBOARD

#define BOARD_NAME          "MyBoard"
#define BOARD_MCU           STM32H743IIT6
#define BOARD_HSE_VALUE     ((uint32_t)25000000)
#define BOARD_LSE_VALUE     ((uint32_t)32768)

/* 外设开关: 1=启用, 0=禁用 */
#define BOARD_HAS_GPIO      1
#define BOARD_HAS_USART1    1
#define BOARD_HAS_USART2    0
#define BOARD_HAS_SDMMC     0
#define BOARD_HAS_USB_CDC   0
#define BOARD_HAS_RTC       0
// ... 按实际外设增删

/* 引脚映射 */
#define BSP_LED_PORT        GPIOH
#define BSP_LED_PIN         GPIO_PIN_7
#define BSP_SHELL_USART     USART1
#define BSP_SHELL_HUART     huart1
#define BSP_SHELL_IRQn      USART1_IRQn

#endif /* BOARD_MYBOARD */
```

### 5. 注册板子

在 `Platform/Board/board_select.h` 中添加一行:

```c
#include "board_myboard.h"   /* 新增 */
```

### 6. 编译

```bash
cmake -B build -DBOARD=MyBoard -G "MinGW Makefiles"
mingw32-make -C build -j8
```

CMake 自动:
- 将 `MyBoard` 转为 C 宏 `-DBOARD_MYBOARD` 激活板级配置
- 添加 `HAL/MyBoard/Inc` 和 `HAL/MyBoard/BSP/` 到头文件路径
- 排除 `HAL/MyBoard/Src/main.c` (使用 `Core/Src/main.c` 作为入口)
- 自动匹配 `HAL/MyBoard/*_FLASH.ld` 作为链接脚本

## CubeMX 代码同步清单

CubeMX 重新生成代码后，需要检查:

| 变更 | 需要同步的文件 |
|------|---------------|
| 新增外设 (如 SPI3) | `hal_init.c` 加 `MX_SPI3_Init()`; `board_xxx.h` 加 `BOARD_HAS_SPIx` |
| 删除外设 | `hal_init.c` 删对应 `MX_xxx_Init()`; `board_xxx.h` 设为 0 |
| 修改时钟树 | `hal_init.c` 复制新的 `SystemClock_Config()` |
| 修改引脚映射 | `board_xxx.h` 更新引脚定义; `bsp_xxx.c` 更新 BSP 驱动 |
| 修改 MPU | `hal_init.c` 复制新的 `MPU_Config()` |

## BSP 驱动开发

板子 BSP 驱动放在 `HAL/MyBoard/BSP/` 下，每个外设一个子目录:

```
BSP/
├── gpio/
│   ├── bsp_gpio.h    # gpio_dev_t 结构体 + gpio_register() 声明
│   └── bsp_gpio.c    # GPIO 读写实现 + gpio_device_init()
├── usart/
│   ├── bsp_usart.h
│   └── bsp_usart.c   # USART 中断接收实现 + usart_device_init()
├── pwm/
├── adc/
├── i2c/
└── cpu/
    ├── bsp_cpu.h
    └── bsp_cpu.c     # CPU 监控驱动 (温度/频率/负载)
```

每个 BSP 驱动实现:
1. 设备结构体 (如 `gpio_dev_t`)
2. `bsp_file_ops_t` 函数表 (open/close/read/write)
3. `xxx_device_init()` 注册函数 (调用 `addDevice()` 将设备注册到 `/dev/`)

参考 `HAL/FK743M2-IIT6/BSP/gpio/` 的实现。

## 工作原理

```
编译时                              运行时
───────                             ──────
cmake -DBOARD=MyBoard               main.c
  │                                   ├─ HAL_BoardInit()      (hal_init.c)
  ├─ -DBOARD_MYBOARD                  ├─ Platform_Init()      (bsp_init.c)
  ├─ HAL/MyBoard/Inc 加入include        │    └─ board_periph_init()
  └─ HAL/MyBoard/ 源码加入编译         └─ MX_FREERTOS_Init()
                                         ├─ DrTInit()
                                         │    └─ devices_init()  (注册外设到 /dev)
                                         └─ ThreadInit()        (创建OS任务)
```
