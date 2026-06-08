# H7OS — STM32H7 实时操作系统 / Real-Time OS

[English](#english) | [中文](#中文)

基于 FreeRTOS + TLSF 的 Linux 风格嵌入式实时操作系统，运行于 STM32H743IIT6 (FK743M2-IIT6)，Cortex-M7 @ 480 MHz。

Linux-style embedded RTOS on STM32H743IIT6, Cortex-M7 @ 480 MHz, FreeRTOS + TLSF.

---

## 目录 / Table of Contents

- [架构 / Architecture](#架构--architecture)
- [快速开始 / Quick Start](#快速开始--quick-start)
- [特性详解 / Features](#特性详解--features)
  - [DrT 设备树与 RAMFS](#drt-设备树与-ramfs)
  - [Shell 命令行](#shell-命令行)
  - [BSP 设备驱动框架](#bsp-设备驱动框架)
  - [SD 卡与 FatFs](#sd-卡与-fatfs)
  - [USB CDC 虚拟串口](#usb-cdc-虚拟串口)
  - [TLSF 内存分配器](#tlsf-内存分配器)
  - [多板型支持](#多板型支持)
- [添加新板子 / Adding a Board](#添加新板子--adding-a-board)
- [添加新设备驱动 / Adding a Device Driver](#添加新设备驱动--adding-a-device-driver)
- [构建 / Build](#构建--build)
- [内存布局 / Memory Layout](#内存布局--memory-layout)
- [启动流程 / Initialization](#启动流程--initialization)
- [文档索引 / Documentation Index](#文档索引--documentation-index)

---

## 架构 / Architecture

```
┌──────────────────────────────────────────────────┐
│ Software 层 (Applications, Command, RAMFS)        │
│   ↕ Platform API only / 仅调用 Platform API       │
├──────────────────────────────────────────────────┤
│ Platform 层 (统一接口, 零板级依赖)                   │
│   ↕                                              │
├──────────────────────────────────────────────────┤
│ HAL 层 (CubeMX 代码 + 板级 BSP, 每板独立目录)       │
└──────────────────────────────────────────────────┘
```

**调用规则**: Software → Platform → HAL。Software 层禁止直接调用 HAL 函数 (如 `HAL_GPIO_WritePin`)，必须通过 Platform API (`dev_read`, `dev_write`, `ram_ls` 等)。

**板级切换**: `cmake -DBOARD=FK743M2-IIT6` 指向 `HAL/FK743M2-IIT6/`，CMake 自动处理头文件路径、源码编译、链接脚本匹配、CubeMX main.c 排除。

详见: [架构文档](docs/01-architecture.md)

---

## 快速开始 / Quick Start

```bash
# 1. 编译 (默认 FK743M2-IIT6)
cmake -B build -G "MinGW Makefiles"
mingw32-make -C build -j8

# 2. 烧录 (ST-Link / DFU)
# 产物: build/H7OS.elf  build/H7OS.hex  build/H7OS.bin

# 3. 连接 USB → 打开串口终端 (115200 8N1)
# 看到提示符: root:/$
```

## 特性详解 / Features

### DrT 设备树与 RAMFS

设备树 (Device Tree) 是 H7OS 的核心，所有设备、文件、挂载点、进程都统一组织为树形结构。

**目录树**:
```
/ (root)
├── dev/              设备节点
│   ├── cpu/CPU        CPU 监控 (温度/频率/负载)
│   ├── serial/USART1  串口 (中断接收, 环形缓冲)
│   ├── gpio/PF7,PH7   GPIO 引脚 (读写 0/1)
│   ├── pwm/PWM2_CH1    PWM 通道 (占空比 0-100%)
│   ├── adc/ADC1        ADC 输入 (电压值)
│   └── i2c/I2C1        I2C 总线 (首字节=从机地址)
├── mnt/               挂载点
│   ├── QSPI            QSPI Flash (16MB W25Qxx, RAMFS 持久化)
│   └── SDcard          SD 卡 (FAT32/exFAT, mount 后挂载)
├── usr/               用户数据 (save 到 QSPI, reboot 恢复)
├── proc/              进程列表 (Shell, TaskMgr, Test, Kernel)
└── bin/               保留
```

**RAMFS**: 内存文件系统，文件最大 127 字节。`save` 命令序列化到 QSPI Flash (CRC32 校验, 魔数 "H7FS")，`reboot` 后 `FS_Deserialize` 恢复。

详见: [文件系统文档](docs/03-filesystem.md)

### Shell 命令行

USB CDC 虚拟串口终端，提示符 `root:/path$`。

**20+ 命令**:

| 类别 | 命令 |
|------|------|
| 文件系统 | `ls` `cd` `pwd` `cat` `echo` `mkdir` `touch` `rm` `cp` `mv` `tree` |
| 设备操作 | `use` (open/close/read/write 设备) |
| 信息 | `info` (设备/任务详情), `help` |
| 挂载 | `mount` (SD 卡) |
| 系统 | `save` `reboot` `reset` `flash` `dfu` |

**示例**:
```bash
use /dev/gpio/PH7 open      # 打开 LED 设备 (状态→Busy)
use /dev/gpio/PH7 write 1   # 点亮 LED
use /dev/gpio/PH7 read      # 读取 '1'
use /dev/gpio/PH7 close     # 释放设备

use /dev/cpu/CPU open read  # CPU: 480MHz, 42.3°C, 5.2% load
use /dev/adc/ADC1 read      # ADC 电压值
use /dev/pwm/PWM2_CH1 write 75  # 75% 占空比

mount /sd                   # 挂载 SD 根目录
cd sd && ls                 # 浏览 SD 卡
cp /usr/config.ini /sd/     # RAMFS → SD 复制
```

详见: [Shell 命令参考](docs/04-shell-commands.md)

### BSP 设备驱动框架

Linux `file_ops` 模型 (`open → read/write → close`)，所有外设统一接口。

**已支持驱动**: GPIO, USART, PWM, ADC, I2C, CPU Monitor

**驱动模式**:
```c
// 每个驱动实现四个回调 + 注册函数
static bsp_file_ops_t xxx_fops = {
    .open  = xxx_open,      // 可空
    .close = xxx_close,     // 可空
    .read  = xxx_read,      // 必需
    .write = xxx_write,     // 可选
};
void xxx_device_init(void) {
    addDevice("dev/xxx", data, "NAME", "desc", TYPE, DEVICE_ON, NULL);
}
```

详见: [BSP 驱动文档](docs/05-bsp-drivers.md)

### SD 卡与 FatFs

- **格式**: FAT32/exFAT (通过 FatFs R0.12c)
- **接口**: SDMMC1 4-bit 模式
- **挂载**: `mount <ramfs_path>` (SD 根目录) 或 `mount <sd_path> <ramfs_path>` (子目录)
- **路径**: 驱动器前缀 `"0:"`, 如 `"0:/dir/file"`
- **操作**: 挂载后 `cd` 进入，`ls`, `mkdir`, `cp`, `mv`, `cat` 等命令透明操作 SD 卡
- **限制**: FatFs 非重入模式 (单任务安全), Tiny 模式 (节省 RAM)

### USB CDC 虚拟串口

- 通过 USB OTG 提供虚拟 COM 口
- Shell 通过 `USB_printf` / `USB_scanf` 交互
- `USB_color_printf(color, fmt, ...)` 支持 ANSI 颜色

### TLSF 内存分配器

Two-Level Segregated Fit, O(1) 分配/释放。两个池运行在 SDRAM:

| 池 | 大小 | API | 用途 |
|----|------|-----|------|
| Kernel | 14 MB | `kernel_alloc/free` | DrT 节点, 文件数据, BSP 驱动 |
| User | 10 MB | `ram_alloc/free` | 用户数据 |

### 多板型支持

HAL → Platform → Software 三层架构, `-DBOARD=xxx` 切换。

```
cmake -DBOARD=FK743M2-IIT6  →  编译 HAL/FK743M2-IIT6/
cmake -DBOARD=MyBoard       →  编译 HAL/MyBoard/
```

---

## 添加新板子 / Adding a Board

6 步流程:

```bash
# 1. 复制模板
cp -r HAL/_template HAL/MyBoard

# 2. CubeMX 生成代码
#    - 打开 CubeMX, 在 HAL/MyBoard/ 创建 MyBoard.ioc
#    - 配置引脚、时钟、外设
#    - Project Manager → Code Generator → 取消 "Generate main()"
#    - 生成代码到 HAL/MyBoard/

# 3. 填充 hal_init.c
#    从 CubeMX 生成的 Src/main.c 复制:
#    - SystemClock_Config() → 完整复制
#    - MPU_Config() → 完整复制
#    - 所有 MX_xxx_Init() 调用 → 按顺序复制到 HAL_BoardInit()

# 4. 创建板级配置
cp Platform/Board/board_template.h Platform/Board/board_myboard.h
# 编辑: 改宏名 BOARD_TEMPLATE → BOARD_MYBOARD
#       设置 BOARD_HAS_xxx 外设开关 (1=启用)

# 5. 注册板子
# 在 Platform/Board/board_select.h 加一行:
#   #include "board_myboard.h"

# 6. 编译
cmake -B build -DBOARD=MyBoard -G "MinGW Makefiles"
mingw32-make -C build -j8
```

**CMake 自动处理**:
- `HAL/MyBoard/Inc` + `HAL/MyBoard/BSP/` + 子目录 → 头文件路径
- `HAL/MyBoard/*.*` → 源码编译 (排除 main.c)
- `HAL/MyBoard/*_FLASH.ld` → 链接脚本
- `-DBOARD_MYBOARD` → C 宏 (连字符自动转下划线)

**CubeMX 同步清单**:
| 变更 | 需同步的文件 |
|------|------------|
| 新增外设 | `hal_init.c` + `board_myboard.h` + `bsp_devices.c` |
| 删除外设 | `hal_init.c` + `board_myboard.h` |
| 修改时钟 | `hal_init.c` SystemClock_Config() |
| 修改 MPU | `hal_init.c` MPU_Config() |
| 修改引脚 | `board_myboard.h` + BSP 驱动 |

详见: [添加新板子完整指南](docs/02-new-board.md)

---

## 添加新设备驱动 / Adding a Device Driver

以 SPI 为例:

```bash
# 1. 创建驱动文件
mkdir HAL/MyBoard/BSP/spi
# 编写 bsp_spi.h (设备结构体 + 注册宏)
# 编写 bsp_spi.c (file_ops 实现 + device_init)

# 2. 注册到系统
# bsp_devices.c 中加入 #include "spi/bsp_spi.h" 和 spi_device_init()

# 3. 添加外设开关
# board_myboard.h: #define BOARD_HAS_SPI 1
# board_select.h: #ifndef BOARD_HAS_SPI → #define BOARD_HAS_SPI 0

# 4. 创建设备树目录
# DrT.c DrTInit() 中: fs_create_child(dev, "spi", 0);
```

详见: [添加新设备驱动完整指南](docs/09-new-device.md)

---

## 构建 / Build

| 项目 | 值 |
|------|-----|
| MCU | STM32H743IIT6, Cortex-M7 |
| 工具链 | arm-none-eabi-gcc 10.3.1 |
| CMake | 4.1+ |
| CubeMX | 6.9.2, FW_H7 1.11.2 |
| C 标准 | C11 |
| 优化 | Debug: -Og -g; Release: -Ofast |

```bash
cmake -B build -DBOARD=FK743M2-IIT6 -G "MinGW Makefiles"
mingw32-make -C build -j8
# 产物: build/H7OS.elf (1.8MB)  build/H7OS.hex (449KB)  build/H7OS.bin (160KB)
```

详见: [构建系统文档](docs/08-build-system.md)

---

## 内存布局 / Memory Layout

| 区域 | 地址 | 大小 | 用途 |
|------|------|------|------|
| FLASH | 0x08000000 | 2 MB | 程序 (~160 KB used) |
| DTCM | 0x20000000 | 128 KB | Cortex-M7 紧耦合数据 |
| RAM_D1 | 0x24000000 | 512 KB | .data .bss heap stack (~83 KB used) |
| SDRAM | 0xC0000000 | 8 MB | LTDC 帧缓冲 |
| SDRAM | 0xC0800000 | 14 MB | 内核 TLSF 池 |
| SDRAM | 0xC1600000 | 10 MB | 用户 TLSF 池 |
| QSPI | 0x90000000 | 16 MB | W25Qxx Flash |

详见: [内存布局文档](docs/07-memory-layout.md)

---

## 启动流程 / Initialization

```
Reset → startup.s → main()
  → HAL_BoardInit()        (MPU → Clock → 全部外设硬件)
  → Platform_Init()         (DFU检测 → USB → SD → Touch)
  → MX_FREERTOS_Init()      (TLSF池 → DrT树 → devices_init → 任务创建)
  → osKernelStart()         (FreeRTOS 调度器)
    → QueueInit: FS_Deserialize → f_mount(SD) → 挂起
    → ShellTask:  scanf → execCMD → print_prompt (循环)
```

详见: [启动流程文档](docs/06-init-flow.md)

---

## 文档索引 / Documentation Index

| # | 文档 | 内容 |
|---|------|------|
| 1 | [架构 / Architecture](docs/01-architecture.md) | 三层架构, 目录树, 调用规则, 板级切换原理 |
| 2 | [添加新板子 / New Board](docs/02-new-board.md) | 6 步流程, CubeMX 同步清单, BSP 模板, 工作原理 |
| 3 | [文件系统 / Filesystem](docs/03-filesystem.md) | DrT 数据结构, 目录树, API 参考, SD 路径转换 |
| 4 | [Shell 命令 / Shell Commands](docs/04-shell-commands.md) | 20+ 命令手册, 设备路径, 读写限制 |
| 5 | [BSP 驱动 / BSP Drivers](docs/05-bsp-drivers.md) | file_ops 模型, GPIO/USART/PWM/ADC/I2C/CPU 详解 |
| 6 | [启动流程 / Init Flow](docs/06-init-flow.md) | 完整启动序列, 任务优先级, 内存初始化时机 |
| 7 | [内存布局 / Memory](docs/07-memory-layout.md) | SDRAM 分区, TLSF 池, FreeRTOS 堆, 链接脚本 |
| 8 | [构建系统 / Build](docs/08-build-system.md) | 工具链, CMake, FatFs 配置, 已知限制 |
| 9 | [添加新驱动 / New Device](docs/09-new-device.md) | 设备驱动开发 (SPI 示例), 驱动模式参考 |
