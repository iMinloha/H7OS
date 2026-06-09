# H7OS — STM32H7 实时操作系统 / Real-Time OS

[English](#english) | [中文](#中文)

基于 FreeRTOS + TLSF 的 Linux 风格嵌入式实时操作系统，运行于 STM32H743IIT6 (FK743M2-IIT6)，Cortex-M7 @ 480 MHz。

Linux-style embedded RTOS on STM32H743IIT6, Cortex-M7 @ 480 MHz, FreeRTOS + TLSF.

---

## 目录 / Table of Contents

- [架构 / Architecture](#架构--architecture)
- [快速开始 / Quick Start](#快速开始--quick-start)
- [特性详解 / Features](#特性详解--features)
  - [Platform 抽象层 / Platform Abstraction Layer](#platform-抽象层--platform-abstraction-layer)
  - [DrT 设备树与 RAMFS](#drt-设备树与-ramfs)
  - [Shell 命令行](#shell-命令行)
  - [BSP 设备驱动框架](#bsp-设备驱动框架)
  - [SD 卡与 FatFs](#sd-卡与-fatfs)
  - [USB CDC 虚拟串口](#usb-cdc-虚拟串口)
  - [TLSF 内存分配器](#tlsf-内存分配器)
  - [多板型支持](#多板型支持)
  - [Builder 脚本引擎 / Builder Script Engine](#builder-脚本引擎--builder-script-engine)
- [添加新板子 / Adding a Board](#添加新板子--adding-a-board)
- [添加新设备驱动 / Adding a Device Driver](#添加新设备驱动--adding-a-device-driver)
- [构建 / Build](#构建--build)
- [内存布局 / Memory Layout](#内存布局--memory-layout)
- [启动流程 / Initialization](#启动流程--initialization)
- [常见问题 / FAQ & Troubleshooting](#常见问题--faq--troubleshooting)
- [文档索引 / Documentation Index](#文档索引--documentation-index)

---

## 架构 / Architecture

```
┌──────────────────────────────────────────────────────────┐
│  Software 层 (Applications, Command, RAMFS)                │
│  仅 #include "platform.h", 绝不直接引用 HAL 头文件           │
│  Only #include "platform.h", NEVER include HAL headers      │
│  调用 / Calls: Platform_GPIO_*, Platform_SD_*, ...         │
├──────────────────────────────────────────────────────────┤
│  Platform 层 (platform.h + platform.c)                      │
│  统一的硬件抽象 API, 零 HAL 类型暴露                          │
│  Unified hardware abstraction API, no HAL types exposed     │
│  + Board/ 目录: 板级配置 (外设开关, 引脚映射, 内存布局)        │
│  + Board/: board config (peripheral switches, pins, memory) │
├──────────────────────────────────────────────────────────┤
│  HAL 层 (CubeMX 代码 + 板级 BSP, 每板独立目录)               │
│  HAL/<Board>/Inc/  +  Src/  +  Startup/  +  BSP/           │
└──────────────────────────────────────────────────────────┘
```

**调用规则 / Call Rule**: Software → Platform → HAL。Software 层禁止直接调用 HAL 函数或引用 HAL 句柄 (如 `HAL_GPIO_WritePin`, `hsd1`, `hadc3`)，必须通过 Platform API。

Software layer MUST NOT call HAL functions directly or reference HAL handles (e.g. `HAL_GPIO_WritePin`, `hsd1`, `hadc3`). All hardware access goes through Platform API.

**板级切换 / Board Switch**: `cmake -DBOARD=FK743M2-IIT6` → 编译 `HAL/FK743M2-IIT6/`。CMake 自动处理头文件路径、源码编译、链接脚本匹配。

`cmake -DBOARD=FK743M2-IIT6` → compiles `HAL/FK743M2-IIT6/`. CMake auto-handles include paths, source compilation, and linker script matching.

### 为什么 Platform/Board/ 下有 .h 文件? / Why are there .h files under Platform/Board/?

`Platform/Board/board_*.h` 是板级**配置**文件 (纯宏定义), 不是 BSP 驱动代码。它们定义:
- 该板子有哪些外设 (`BOARD_HAS_SDMMC`, `BOARD_HAS_LTDC`, ...)
- MCU 型号与时钟参数
- 引脚映射和外部存储器布局

**为什么不在 BSP 里?** BSP (HAL/<板名>/BSP/) 负责"怎么驱动"每个外设; Board 配置负责"有哪些"外设。分离"有什么"和"怎么做"使得:
1. 编译时就知道哪些代码路径生效 (`#if BOARD_HAS_SDMMC`)
2. Platform 层无需依赖 HAL 目录即可获取板级信息
3. 添加新板子只需复制一份配置 + 一份 BSP 实现

类比 Linux 内核: `Platform/Board/` ≈ Device Tree Source (`.dts`); `HAL/<板名>/BSP/` ≈ 设备驱动。

`Platform/Board/board_*.h` files are board **configuration** (pure macros), NOT BSP driver code. They define WHAT peripherals a board has. BSP (HAL/<board>/BSP/) defines HOW to drive them. Separating "what" from "how" is the same pattern as Linux's device tree (`.dts`) vs. device drivers.

详见: [架构文档](docs/01-architecture.md)

---

## 快速开始 / Quick Start

```bash
# 1. 编译 (默认 FK743M2-IIT6) / Build (default FK743M2-IIT6)
cmake -B build -G "MinGW Makefiles"
mingw32-make -C build -j8

# 2. 烧录 / Flash (ST-Link or DFU)
# 产物 / Artifacts: build/H7OS.elf  build/H7OS.hex  build/H7OS.bin

# 3. 连接 USB → 打开串口终端 (115200 8N1) / Connect USB → Open serial terminal
# 看到提示符 / See prompt: root:/$
```

---

## 特性详解 / Features

### Platform 抽象层 / Platform Abstraction Layer

Platform 是 Software 层访问硬件的**唯一入口**。所有 HAL 句柄对 Software 不可见。

Platform is the **ONLY entry point** for Software to access hardware. All HAL handles are invisible to Software.

**API 概览 / API Overview**:

| 类别 / Category | 函数 / Functions | 说明 / Description |
|---|---|---|
| System | `Platform_Init()`, `Platform_Reset()`, `Platform_GetSysClockFreq()` | 系统初始化与信息 |
| CPU | `Platform_CPU_TempInit()`, `Platform_CPU_GetTemperature()` | 内部温度传感器 |
| GPIO | `Platform_GPIO_WritePin()`, `ReadPin()`, `TogglePin()` | 引脚读写翻转 |
| SD Card | `Platform_SD_IsPresent()`, `GetCapacity()`, `GetInfo()`, `GetFatFS()`, `GetPath()` | SD 卡检测与信息 |
| QSPI | `Platform_QSPI_Init()`, `ReadID()`, `ChipErase()`, `Write()`, `Read()` | W25Qxx Flash 操作 |
| RTC/DFU | `Platform_DFU_Request()`, `SetFlag()`, `ClearFlag()` | DFU 固件升级 |
| Board | `Platform_GetBoardName()`, `Platform_HasPeripheral()` | 板级信息查询 |
| Device | `Platform_DevicesInit()` | 注册外设到 DrT |

```c
// 使用示例 / Usage example
#include "platform.h"

// GPIO: 翻转 LED / Toggle LED
Platform_GPIO_TogglePin((void *)GPIOH, GPIO_PIN_7);

// SD 卡检测 / SD card detection
if (Platform_SD_IsPresent()) {
    uint64_t cap = Platform_SD_GetCapacity();
    printf("SD: %lld MB\n", (long long)(cap / 1024 / 1024));
}

// QSPI: 擦除 + 写入 / Erase + Write
Platform_QSPI_ChipErase();
Platform_QSPI_Write(data, 0, size);

// DFU: 进入固件升级模式 / Enter DFU mode
Platform_DFU_Request();  // 保存 + 复位 / Save + Reset
```

### DrT 设备树与 RAMFS

设备树 (Device Tree) 是 H7OS 的核心，所有设备、文件、挂载点、进程都统一组织为树形结构。

**目录树 / Directory Tree**:
```
/ (root)
├── dev/              设备节点 / Device nodes
│   ├── cpu/CPU        CPU 监控 (温度/频率/负载)
│   ├── serial/USART1  串口 (中断接收, 环形缓冲)
│   ├── gpio/PF7,PH7   GPIO 引脚 (读写 0/1)
│   ├── pwm/PWM2_CH1    PWM 通道 (占空比 0-100%)
│   ├── adc/ADC1        ADC 输入 (电压值)
│   └── i2c/I2C1        I2C 总线 (首字节=从机地址)
├── mnt/               挂载点 / Mount points
│   ├── QSPI            QSPI Flash (16MB W25Qxx, RAMFS 持久化)
│   └── SDcard          SD 卡 (FAT32/exFAT, mount 后挂载)
├── usr/               用户数据 / User data (save → QSPI, reboot 恢复)
├── proc/              进程列表 / Process list (Shell, TaskMgr, Test, Kernel)
└── bin/               保留 / Reserved
```

**RAMFS**: 内存文件系统，文件最大 127 字节。`save` 命令序列化到 QSPI Flash (CRC32 校验, 魔数 "H7FS")，`reboot` 后 `FS_Deserialize` 恢复。

In-memory filesystem, max 127 bytes per file. `save` serializes to QSPI Flash (CRC32, magic "H7FS"), restored on reboot via `FS_Deserialize`.

详见: [文件系统文档](docs/03-filesystem.md)

### Shell 命令行

USB CDC 虚拟串口终端，提示符 `root:/path$`。

**20+ 命令 / Commands**:

| 类别 / Category | 命令 / Commands |
|---|---|
| 文件系统 / FS | `ls` `cd` `pwd` `cat` `echo` `mkdir` `touch` `rm` `cp` `mv` `tree` |
| 设备操作 / Device | `use` (open/close/read/write 设备) |
| 信息 / Info | `info` (设备/任务详情), `help` |
| 挂载 / Mount | `mount` (SD 卡) |
| 系统 / System | `save` `reboot` `reset` `flash` `dfu` |

**示例 / Examples**:
```bash
use /dev/gpio/PH7 open      # 打开 LED 设备 / Open LED device
use /dev/gpio/PH7 write 1   # 点亮 LED / Turn LED on
use /dev/gpio/PH7 read      # 读取 '1' / Read '1'
use /dev/gpio/PH7 close     # 释放设备 / Release device

use /dev/cpu/CPU open read  # CPU: 480MHz, 42.3°C, 5.2% load
use /dev/adc/ADC1 read      # ADC 电压值 / ADC voltage
use /dev/pwm/PWM2_CH1 write 75  # 75% 占空比 / 75% duty

mount /sd                   # 挂载 SD 根目录 / Mount SD root
cd sd && ls                 # 浏览 SD 卡 / Browse SD card
cp /usr/config.ini /sd/     # RAMFS → SD 复制 / RAMFS → SD copy
```

详见: [Shell 命令参考](docs/04-shell-commands.md)

### BSP 设备驱动框架

Linux `file_ops` 模型 (`open → read/write → close`)，所有外设统一接口。

Linux-style `file_ops` model, unified interface for all peripherals.

**已支持驱动 / Supported Drivers**: GPIO, USART, PWM, ADC, I2C, CPU Monitor

**驱动模式 / Driver Pattern**:
```c
// 每个驱动实现四个回调 + 注册函数
// Each driver implements 4 callbacks + register function
static bsp_file_ops_t xxx_fops = {
    .open  = xxx_open,      // 可空 / optional
    .close = xxx_close,     // 可空 / optional
    .read  = xxx_read,      // 必需 / required
    .write = xxx_write,     // 可选 / optional
};
void xxx_device_init(void) {
    addDevice("dev/xxx", data, "NAME", "desc", TYPE, DEVICE_ON, NULL);
}
```

详见: [BSP 驱动文档](docs/05-bsp-drivers.md)

### SD 卡与 FatFs / SD Card & FatFs

- **格式 / Format**: FAT32/exFAT (FatFs R0.12c)
- **接口 / Interface**: SDMMC1 4-bit 模式
- **挂载 / Mount**: `mount <ramfs_path>` (SD 根目录) 或 `mount <sd_path> <ramfs_path>` (子目录)
- **路径 / Path**: 驱动器前缀 `"0:"`, 如 `"0:/dir/file"`
- **操作 / Operations**: 挂载后 `cd` 进入，`ls`, `mkdir`, `cp`, `mv`, `cat` 等命令透明操作 SD 卡
- **限制 / Limitation**: FatFs 非重入模式 (单任务安全), Tiny 模式 (节省 RAM)

> ⚠️ **SD 卡常见问题 / SD Card Troubleshooting** — 见下方 [常见问题](#常见问题--faq--troubleshooting) 章节。

### USB CDC 虚拟串口

- USB OTG 虚拟 COM 口 / Virtual COM port via USB OTG
- Shell 通过 `USB_printf` / `USB_scanf` 交互
- `USB_color_printf(color, fmt, ...)` 支持 ANSI 颜色 / ANSI color support

### TLSF 内存分配器

Two-Level Segregated Fit, O(1) 分配/释放。两个池运行在 SDRAM:

| 池 / Pool | 大小 / Size | API | 用途 / Purpose |
|---|---|---|---|
| Kernel | 14 MB | `kernel_alloc/free` | DrT 节点, 文件数据, BSP 驱动 |
| User | 10 MB | `ram_alloc/free` | 用户数据 / User data |

### 多板型支持 / Multi-Board Support

HAL → Platform → Software 三层架构, `-DBOARD=xxx` 切换。

```
cmake -DBOARD=FK743M2-IIT6  →  编译 HAL/FK743M2-IIT6/
cmake -DBOARD=MyBoard       →  编译 HAL/MyBoard/
```

---

### Builder 脚本引擎 / Builder Script Engine

异步嵌入式脚本解释器，独立 FreeRTOS 线程执行，不阻塞 Shell。支持 `for`/`while`/`if` 控制流、`open`/`read`/`write`/`close` 设备 I/O、变量和算术运算。

```python
# GPIO 闪烁
val = 0
while 1
  write /dev/gpio/PF7 val
  if val == 0
     val = 1
  else
     val = 0
  endif
  delay(500)
endwhile
```

| 命令 | 说明 |
|------|------|
| `run <file>` | 异步启动脚本 (RAMFS 或 SD) |
| `kill <name\|pid>` | 终止脚本 |
| `info /proc/<name>` | 查看脚本任务状态 |

**语法**: 变量赋值、`+` `-` `*` `/` `++` `--` 算术、`==` `!=` `>` `<` `>=` `<=` 比较、`0xNN` 十六进制字节。所有 Shell 命令可直接在脚本中调用。

**外设 I/O**: 统一 `open` → `read`/`write` → `close` 接口，支持 GPIO / USART / PWM / ADC / I2C / CPU。脚本引擎自动 `open`，直接调用软件层 `dev_*` 函数。

详见: [Builder 脚本引擎文档](docs/10-script-engine.md)

---

## 添加新板子 / Adding a Board

6 步流程 / 6-step process:

```bash
# 1. 复制模板 / Copy template
cp -r HAL/_template HAL/MyBoard

# 2. CubeMX 生成代码 / Generate CubeMX code
#    - 打开 CubeMX, 在 HAL/MyBoard/ 创建 MyBoard.ioc
#    - 配置引脚、时钟、外设
#    - Project Manager → Code Generator → 取消 "Generate main()"
#    - 生成代码到 HAL/MyBoard/

# 3. 填充 hal_init.c / Fill hal_init.c
#    从 CubeMX 生成的 Src/main.c 复制:
#    - SystemClock_Config() → 完整复制
#    - MPU_Config() → 完整复制
#    - 所有 MX_xxx_Init() 调用 → 按顺序复制到 HAL_BoardInit()

# 4. 创建板级配置 / Create board config
cp Platform/Board/board_template.h Platform/Board/board_myboard.h
# 编辑: 改宏名 BOARD_TEMPLATE → BOARD_MYBOARD
#       设置 BOARD_HAS_xxx 外设开关 (1=启用)

# 5. 注册板子 / Register board
# 在 Platform/Board/board_select.h 加一行:
#   #include "board_myboard.h"

# 6. 编译 / Build
cmake -B build -DBOARD=MyBoard -G "MinGW Makefiles"
mingw32-make -C build -j8
```

**CMake 自动处理 / CMake Auto-handling**:
- `HAL/MyBoard/Inc` + `HAL/MyBoard/BSP/` + 子目录 → 头文件路径 / include paths
- `HAL/MyBoard/*.*` → 源码编译 (排除 main.c) / source compilation (excl. main.c)
- `HAL/MyBoard/*_FLASH.ld` → 链接脚本 / linker script
- `-DBOARD_MYBOARD` → C 宏 (连字符自动转下划线 / hyphens → underscores)

**CubeMX 同步清单 / CubeMX Sync Checklist**:
| 变更 / Change | 需同步的文件 / Files to sync |
|---|---|
| 新增外设 / New peripheral | `hal_init.c` + `board_myboard.h` + `bsp_devices.c` |
| 删除外设 / Remove peripheral | `hal_init.c` + `board_myboard.h` |
| 修改时钟 / Clock change | `hal_init.c` SystemClock_Config() |
| 修改 MPU / MPU change | `hal_init.c` MPU_Config() |
| 修改引脚 / Pin change | `board_myboard.h` + BSP 驱动 |

详见: [添加新板子完整指南](docs/02-new-board.md) / [Complete New Board Guide](docs/02-new-board.md)

---

## 添加新设备驱动 / Adding a Device Driver

以 SPI 为例 / Using SPI as example:

```bash
# 1. 创建驱动文件 / Create driver files
mkdir HAL/MyBoard/BSP/spi
# 编写 bsp_spi.h (设备结构体 + 注册宏)
# 编写 bsp_spi.c (file_ops 实现 + device_init)

# 2. 注册到系统 / Register to system
# bsp_devices.c 中加入 #include "spi/bsp_spi.h" 和 spi_device_init()

# 3. 添加外设开关 / Add peripheral switch
# board_myboard.h: #define BOARD_HAS_SPI 1
# board_select.h: #ifndef BOARD_HAS_SPI → #define BOARD_HAS_SPI 0

# 4. 创建设备树目录 / Create device tree directory
# DrT.c DrTInit() 中: fs_create_child(dev, "spi", 0);
```

详见: [添加新设备驱动完整指南](docs/09-new-device.md) / [Complete New Device Driver Guide](docs/09-new-device.md)

---

## 构建 / Build

| 项目 / Item | 值 / Value |
|---|---|
| MCU | STM32H743IIT6, Cortex-M7 |
| 工具链 / Toolchain | arm-none-eabi-gcc 10.3.1 |
| CMake | 4.1+ |
| CubeMX | 6.9.2, FW_H7 1.11.2 |
| C 标准 / C Standard | C11 |
| 优化 / Optimization | Debug: -Og -g; Release: -Ofast |

```bash
cmake -B build -DBOARD=FK743M2-IIT6 -G "MinGW Makefiles"
mingw32-make -C build -j8
# 产物 / Artifacts: build/H7OS.elf (1.8MB)  build/H7OS.hex (449KB)  build/H7OS.bin (160KB)
```

详见: [构建系统文档](docs/08-build-system.md) / [Build System Doc](docs/08-build-system.md)

---

## 内存布局 / Memory Layout

| 区域 / Region | 地址 / Address | 大小 / Size | 用途 / Purpose |
|---|---|---|---|
| FLASH | 0x08000000 | 2 MB | 程序 (~160 KB used) |
| DTCM | 0x20000000 | 128 KB | Cortex-M7 紧耦合数据 |
| RAM_D1 | 0x24000000 | 512 KB | .data .bss heap stack (~83 KB used) |
| SDRAM (Video) | 0xC0000000 | 8 MB | LTDC 帧缓冲 / Frame buffer |
| SDRAM (Kernel) | 0xC0800000 | 14 MB | 内核 TLSF 池 / Kernel TLSF pool |
| SDRAM (User) | 0xC1600000 | 10 MB | 用户 TLSF 池 / User TLSF pool |
| QSPI | 0x90000000 | 16 MB | W25Qxx Flash |

详见: [内存布局文档](docs/07-memory-layout.md) / [Memory Layout Doc](docs/07-memory-layout.md)

---

## 启动流程 / Initialization

```
Reset → startup.s → main()
  → HAL_BoardInit()         (MPU → Clock → 全部外设硬件)
                              (MPU → Clock → all peripheral hardware)
  → Platform_Init()          (DFU检测 → USB → SD → Touch)
                              (DFU check → USB → SD → Touch)
  → MX_FREERTOS_Init()       (TLSF池 → DrT树 → devices_init → 任务创建)
                              (TLSF pool → DrT tree → devices_init → task creation)
  → osKernelStart()          (FreeRTOS 调度器 / Scheduler)
    → QueueInit: FS_Deserialize → f_mount(SD) → 挂起 / Suspend
    → ShellTask:  scanf → execCMD → print_prompt (循环 / Loop)
```

详见: [启动流程文档](docs/06-init-flow.md) / [Init Flow Doc](docs/06-init-flow.md)

---

## 常见问题 / FAQ & Troubleshooting

### SD 卡无法挂载 / SD Card Mount Failure

**现象 / Symptom**: `mount /sd` 后无法访问 SD 卡，或输出 `SD Init Failed`。

**Symptoms**: After `mount /sd`, SD card is inaccessible, or `SD Init Failed` is printed.

**常见原因与解决方法 / Common Causes & Solutions**:

| 问题 / Issue | 原因 / Cause | 解决 / Solution |
|---|---|---|
| 找不到 SD 卡 / SD not found | SD 卡未插入或接触不良 / Card not inserted or poor contact | 重新插拔 SD 卡 / Re-insert SD card |
| 挂载失败 / Mount failed | SD 卡文件系统损坏或非 FAT32/exFAT / Corrupted FS or not FAT32/exFAT | 系统会自动尝试 `f_mkfs` 格式化; 如果失败, 用 PC 格式化为 FAT32 / System auto-tries `f_mkfs`; if fails, format to FAT32 on PC |
| 格式化失败 / Format failed | MDMA 未使能 (FatFs 的 `f_mkfs` 需要 MDMA) / MDMA not enabled | 确保 CubeMX 中 MDMA 已启用, 且 `hal_init.c` 的 `HAL_BoardInit()` 中 `MX_MDMA_Init()` 在 `MX_SDMMC1_SD_Init()` 之前调用 / Ensure MDMA is enabled in CubeMX, and `MX_MDMA_Init()` is called before `MX_SDMMC1_SD_Init()` in `hal_init.c` |
| database 字段错误 / Wrong database field | FatFs 已知 bug: FAT32 的 `database` 未正确计算 / Known FatFs bug: FAT32 `database` not correctly calculated | 系统自动修复 (启动日志可见 `fix database` 消息) / System auto-fixes (see `fix database` in boot log) |
| SD 卡容量为 0 / SD capacity is 0 | SD 卡未正确初始化 (SDMMC 时钟或引脚问题) / SD not properly initialized (SDMMC clock or pin issue) | 检查 CubeMX 中 SDMMC1 配置 (4-bit mode, CLK ≤ 25MHz for init) / Check SDMMC1 config in CubeMX |
| 重启后 SD 挂载丢失 / SD mount lost after reboot | RAMFS 的 SD 挂载点未正确持久化 / RAMFS SD mount point not properly persisted | `save` 命令在 `reboot` 前执行会自动保存挂载点信息 / `save` command before `reboot` auto-saves mount info |

**调试步骤 / Debug Steps**:

1. 检查启动日志中 `[xTaskInit]` 消息:
   - `SD Card Not Found` → SD 卡硬件问题 / Hardware issue
   - `SD Card Capacity: XXXXMB` → SD 卡已检测到 / SD card detected
2. 检查 `Fatfs Succeed` 或 `Fatfs Format Succeed` → 文件系统就绪 / Filesystem ready
3. 如果看到 `fix database` → 正常, 是已知修复 / Normal, known fix applied
4. 如果看到 `SD Init Failed` → 尝试更换 SD 卡或在 PC 上格式化为 FAT32 / Try another SD card or format to FAT32 on PC

**初始化顺序要求 / Init Order Requirements**:

```
MDMA_Init() → SDMMC_Init() → FATFS_Init() → f_mount()
    ↑                                            ↑
  必须先于 SDMMC                          必须在 osKernelStart() 之后
  Must be before SDMMC                    Must be after osKernelStart()
                                           (FatFs uses FreeRTOS queues)
```

### 设备树出现重复节点 / Duplicate Device Tree Nodes

**现象 / Symptom**: `tree` 命令显示每个设备出现两次 (如两个 `CPU`, 两个 `USART1`)。

**原因 / Cause**: `freertos.c` 中 `devices_init()` 被调用了两次 — 一次在 `DrTInit()` 内部, 一次在 `taskGlobalInit()` 之后显式调用。

**解决 / Solution**: 移除 `freertos.c` 中重复的 `devices_init()` 调用。当前版本已修复。

### 如何确定某个外设是否可用 / How to Check if a Peripheral is Available

```c
#include "platform.h"

if (Platform_HasPeripheral("SDMMC")) {
    // SD 卡操作 / SD card operations
}
if (Platform_HasPeripheral("LTDC")) {
    // 显示操作 / Display operations
}
```

---

## 文档索引 / Documentation Index

| # | 文档 / Doc | 内容 / Content |
|---|---|---|
| 1 | [架构 / Architecture](docs/01-architecture.md) | 三层架构, 目录树, 调用规则, 板级切换原理, Platform API 设计 |
| 2 | [添加新板子 / New Board](docs/02-new-board.md) | 6 步流程, CubeMX 同步清单, BSP 模板, 工作原理 |
| 3 | [文件系统 / Filesystem](docs/03-filesystem.md) | DrT 数据结构, 目录树, API 参考, SD 路径转换 |
| 4 | [Shell 命令 / Shell Commands](docs/04-shell-commands.md) | 20+ 命令手册, 设备路径, 读写限制 |
| 5 | [BSP 驱动 / BSP Drivers](docs/05-bsp-drivers.md) | file_ops 模型, GPIO/USART/PWM/ADC/I2C/CPU 详解 |
| 6 | [启动流程 / Init Flow](docs/06-init-flow.md) | 完整启动序列, 任务优先级, 内存初始化时机 |
| 7 | [内存布局 / Memory](docs/07-memory-layout.md) | SDRAM 分区, TLSF 池, FreeRTOS 堆, 链接脚本 |
| 8 | [构建系统 / Build](docs/08-build-system.md) | 工具链, CMake, FatFs 配置, 已知限制 |
| 9 | [添加新驱动 / New Device](docs/09-new-device.md) | 设备驱动开发 (SPI 示例), 驱动模式参考 |
| 10 | [脚本引擎 / Script Engine](docs/10-script-engine.md) | Builder 脚本解释器, for/if/变量, run 命令 |
