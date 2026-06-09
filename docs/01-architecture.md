# H7OS 系统架构 / Architecture

## 三层架构 / Three-Layer Architecture

```
┌──────────────────────────────────────────────────────────────┐
│ Software 层 / Software Layer                                 │
│ Applications + Command + RAMFS (DrT) + FATFS                 │
│ OS: device tree, RAM filesystem, shell, tasks                │
│ OS: 设备树, 内存文件系统, 命令行, 任务管理                       │
│                                                              │
│ 仅 #include "platform.h" / Only #include "platform.h"        │
├──────────────────────────────────────────────────────────────┤
│ Platform 层 / Platform Layer                                 │
│ platform.h + platform.c + Board/ + bsp_file_ops.h            │
│                                                              │
│ 统一的硬件抽象 API, 不暴露任何 HAL 类型                          │
│ Unified hardware abstraction API, no HAL types exposed        │
│                                                              │
│ API: Platform_GPIO_*, Platform_SD_*, Platform_QSPI_*, ...    │
├──────────────────────────────────────────────────────────────┤
│ HAL 层 / HAL Layer                                           │
│ CubeMX generated code + board BSP drivers                    │
│ CubeMX 生成代码 + 板级 BSP 驱动                                 │
│ One folder per board / 每块板子一个独立目录                      │
└──────────────────────────────────────────────────────────────┘
```

### 调用规则 / Call Rules

```
Software → Platform → HAL        ✅ allowed / 允许
Software → HAL                   ❌ forbidden / 禁止
Platform → HAL                   ✅ allowed / 允许
HAL → Platform                   ❌ forbidden / 禁止
HAL → Software                   ❌ forbidden / 禁止
```

**Software 层规则 / Software Layer Rules**:

Software 层**只能** `#include "platform.h"`，**不能**直接引用:
- HAL 头文件 (`stm32h7xx_hal.h`, `main.h`, `adc.h`, `sdmmc.h`, `quadspi.h`, ...)
- HAL 句柄 (`hsd1`, `hadc3`, `hqspi`, `huart1`, `htim2`, ...)
- HAL 函数 (`HAL_GPIO_WritePin`, `HAL_ADC_Start`, `HAL_SD_GetCardInfo`, ...)

Software layer **MUST ONLY** `#include "platform.h"` and **MUST NOT** directly reference any HAL headers, handles, or functions.

**Platform API 是 Software ↔ 硬件的唯一桥梁。**
**Platform API is the ONLY bridge between Software and hardware.**

示例 / Example:
```c
// ❌ 错误 / Wrong — Software 层直接调 HAL
#include "gpio.h"
HAL_GPIO_TogglePin(GPIOH, GPIO_PIN_7);

// ✅ 正确 / Correct — 通过 Platform API
#include "platform.h"
Platform_GPIO_TogglePin((void *)GPIOH, GPIO_PIN_7);  // 或使用 BSP_LED_PORT
Platform_GPIO_TogglePin((void *)BSP_LED_PORT, BSP_LED_PIN);
```

### Platform API 设计原则 / Platform API Design Principles

1. **不暴露 HAL 类型 / No HAL types exposed**: `Platform_SD_Info` 替代 `HAL_SD_CardInfoTypeDef`
2. **不暴露 HAL 句柄 / No HAL handles exposed**: `Platform_SD_GetCapacity()` 替代 `HAL_SD_GetCardInfo(&hsd1, ...)`
3. **全部用 `void*` 传递硬件指针 / All hardware pointers passed as `void*`**
4. **条件编译透明 / Conditional compilation transparent**: 外设不存在时 API 返回 0/-1/空指针

**为什么 Platform/Board/ 下有 .h 文件? / Why .h files under Platform/Board/?**

`Platform/Board/board_*.h` 是板级**配置** (纯宏定义), 不是 BSP 驱动:
- **Board config (Platform/Board/)**: 定义"有什么" — 外设开关, 引脚映射, 内存布局
- **BSP drivers (HAL/<board>/BSP/)**: 实现"怎么做" — 每个外设的 file_ops 驱动

类比 Linux: `board_*.h` ≈ Device Tree (`.dts`); `BSP/` ≈ 设备驱动 (`.c`)。

### 目录树 / Directory Tree

```
H7OS/
├── HAL/                          # HAL layer / HAL 层
│   ├── _template/                #   template for new boards / 新板子模板
│   └── FK743M2-IIT6/             #   FK743M2-IIT6 dev board / 开发板
│       ├── Inc/                  #     CubeMX headers / CubeMX 头文件
│       ├── Src/                  #     CubeMX sources / CubeMX 源文件
│       ├── Startup/              #     startup assembly / 启动汇编
│       ├── BSP/                  #     board BSP drivers / 板级 BSP 驱动
│       │   ├── hal_init.c        #       HAL_BoardInit() entry / 统一入口
│       │   ├── bsp_devices.c     #       devices_init + board_periph_init
│       │   ├── gpio/             #       GPIO driver / GPIO 驱动
│       │   ├── usart/            #       USART driver / 串口驱动
│       │   ├── pwm/              #       PWM driver / PWM 驱动
│       │   ├── adc/              #       ADC driver / ADC 驱动
│       │   ├── i2c/              #       I2C driver / I2C 驱动
│       │   ├── cpu/              #       CPU monitor / CPU 监控
│       │   ├── lcd/              #       LCD panel config / 液晶配置
│       │   └── touch/            #       touch screen / 触摸屏
│       ├── Drivers/              #     CubeMX HAL 库 (CMSIS + HAL_Driver)
│       │   ├── CMSIS/            #       CMSIS 核心头文件
│       │   └── STM32H7xx_HAL_Driver/  #  STM32 HAL 驱动
│       ├── *.ioc                 #     CubeMX project / CubeMX 工程
│       └── *.ld                  #     linker script / 链接脚本
│
├── Platform/                     # Platform layer / Platform 层
│   ├── platform.h                #   ⭐ THE ONE HEADER for Software / Software 唯一头文件
│   ├── platform.c                #   ⭐ Implementation wrapping all HAL / 封装所有 HAL 的实现
│   ├── bsp_file_ops.h            #   file_ops interface (open/close/read/write)
│   ├── dev_register.h            #   统一设备注册 (addDevice+loadDevice+fops)
│   ├── script_config.h           #   Builder 脚本引擎配置
│   ├── bsp_devices.h             #   Internal: device registration / 内部: 设备注册 API
│   ├── bsp_init.h/.c             #   Platform_Init() declaration + impl
│   └── Board/                    #   board configs / 板级配置
│       ├── board_select.h        #     board selector / 板级选择器
│       ├── board_fk743m2.h       #     FK743M2 config / FK743M2 配置
│       └── board_template.h      #     template for new boards / 模板
│
├── Core/                         # App entry / 应用入口
│   ├── Inc/FreeRTOSConfig.h      #   FreeRTOS config / FreeRTOS 配置
│   └── Src/                      #   main.c, freertos.c, syscalls, sysmem
│
├── Applications/                 # RTOS tasks / RTOS 任务
│   ├── inc/                      #   task headers / 任务头文件
│   └── src/                      #   task implementations / 任务实现
│       ├── xTaskInit.c           #     global init / 全局初始化 (uses Platform API)
│       ├── xShellTask.c          #     USB CDC shell / USB CDC 终端
│       ├── xTaskManager.c        #     CPU load monitor / CPU 负载监控
│       └── xNoneTask.c           #     idle LED blink / 空闲 LED 闪烁 (uses Platform_GPIO)
│
├── Command/                      # Shell commands / Shell 命令
│   ├── Register.h/.c             #   command registry / 命令注册表
│   ├── ls/ cd/ pwd/ cat/ echo/   #   filesystem commands / 文件系统命令
│   ├── mkdir/ touch/ rm/ cp/ mv/ #   file ops / 文件操作
│   ├── mount/ tree/ use/ info/   #   system commands / 系统命令
│   ├── help/ save/ reboot/       #   utility / 工具
│   └── reset/ flash/ dfu/        #   maintenance / 维护 (uses Platform API)
│
├── RAMFS/                        # DrT device tree + RAM filesystem
│   └── init.d/
│       ├── Core/DrT.h, DrT.c     #   core data structures / 核心数据结构
│       ├── Periph/cpu.c          #   CPU info (uses Platform API)
│       ├── Periph/FS_Serial.c    #   filesystem persistence (uses Platform_QSPI_*)
│       └── ...
│
├── FATFS/                        # FatFs integration / FatFs 集成
├── USB_DEVICE/                   # USB CDC virtual COM / USB CDC 虚拟串口
├── Drivers/                      # OS Kernel (TLSF allocator) / 内核
├── Middlewares/                  # FreeRTOS + FatFs + USB / 中间件
├── docs/                         # documentation / 文档
└── CMakeLists.txt                # build system / 构建系统
```

### 板级切换 / Board Switching

```
cmake -DBOARD=FK743M2-IIT6 →  HAL/FK743M2-IIT6/ 编译
cmake -DBOARD=MyBoard     →  HAL/MyBoard/     编译
```

CMake 自动 / CMake auto:
- 添加 `HAL/${BOARD}/Inc` 和 `HAL/${BOARD}/BSP` 到头文件路径 / Add to include path
- 编译 `HAL/${BOARD}/` 下所有源码 / Compile all sources under HAL/${BOARD}/
- 排除 CubeMX 生成的 `HAL/${BOARD}/Src/main.c` / Exclude CubeMX main.c
- 匹配 `HAL/${BOARD}/*_FLASH.ld` 作为链接脚本 / Match linker script
- 生成 `-DBOARD_XXX` 宏激活对应板级配置 / Generate board selection macro

### 关键文件依赖 / Key File Dependencies

```
Platform/Board/board_select.h
  └── board_fk743m2.h (or board_xxx.h)
        └── #define BOARD_HAS_SDMMC 1 ...

Platform/platform.h            ← Software 层唯一 include / Software's only include
  ├── bsp_file_ops.h           ← file_ops 类型 / file_ops type
  └── board_select.h           ← BOARD_NAME, BSP_LED_PORT, ...

Platform/platform.c            ← 封装所有 HAL 调用 / Wraps all HAL calls
  ├── HAL 头文件 / HAL headers  ← main.h, adc.h, sdmmc.h, quadspi.h, ...
  └── bsp_devices.h            ← devices_init(), board_periph_init()

HAL/<board>/BSP/bsp_devices.c  ← 实现板级外设注册 / Implements board device registration
  ├── board_select.h           ← #if BOARD_HAS_SDMMC 条件编译
  └── 各 BSP 驱动 / BSP drivers ← gpio/bsp_gpio.h, adc/bsp_adc.h, ...
```
