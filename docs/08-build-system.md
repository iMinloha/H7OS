# 构建系统 / Build System

## 工具链 / Toolchain

| 组件 / Component | 版本 / Version |
|---|---|
| arm-none-eabi-gcc | 10.3.1 (GNU Arm Embedded) |
| CMake | 4.1+ |
| MinGW Make | Windows |
| STM32CubeMX | 6.9.2, FW_H7 1.11.2 |

## 编译 / Build

```bash
# 默认板子 (FK743M2-IIT6)
cmake -B build -G "MinGW Makefiles"
mingw32-make -C build -j8

# 指定板子 / specify board
cmake -B build -DBOARD=FK743M2-IIT6 -G "MinGW Makefiles"
mingw32-make -C build -j8

# Release 优化 / release build
cmake -B build -DBOARD=FK743M2-IIT6 -DCMAKE_BUILD_TYPE=Release -G "MinGW Makefiles"
```

## 编译选项 / Compile Flags

| 选项 / Flag | 说明 / Description |
|---|---|
| `-mcpu=cortex-m7 -mthumb` | Cortex-M7 目标 |
| `-mfloat-abi=hard -mfpu=fpv4-sp-d16` | 硬件浮点 / HW float |
| `-DSTM32H743xx -DUSE_HAL_DRIVER` | MCU 定义 |
| `-DBOARD_FK743M2_IIT6` | 板子选择 (CMake 自动) |
| `-ffunction-sections -fdata-sections` | 按需链接 / dead code elimination |
| `-Og -g` (Debug) | 最小优化 + 调试信息 |
| `-Ofast` (Release) | 最大速度优化 |

## 链接器 / Linker

| 选项 / Flag | 说明 / Description |
|---|---|
| `-Wl,-gc-sections` | 移除未使用段 / remove unused sections |
| `--print-memory-usage` | 打印内存占用 |
| `-Map=<name>.map` | 生成 Map 文件 |

## 产物 / Output

| 文件 / File | 说明 / Description |
|---|---|
| `build/H7OS.elf` | ELF 可执行文件 (含调试信息) |
| `build/H7OS.hex` | Intel Hex (烧录用 / for flashing) |
| `build/H7OS.bin` | 原始二进制 (DFU 用 / for DFU) |
| `build/H7OS.map` | 内存布局 Map 文件 |

## 添加新板子 / Adding a Board

详见 [`docs/02-new-board.md`](02-new-board.md)。简要流程:

1. `mkdir HAL/MyBoard` → CubeMX 生成代码
2. 复制 `Platform/Board/board_template.h` → 创建配置
3. 在 `board_select.h` 注册
4. `cmake -DBOARD=MyBoard`

## FatFs 配置 / FatFs Configuration

`FATFS/Target/ffconf.h`:

| 配置 / Config | 值 / Value | 说明 |
|---|---|---|
| _VOLUMES | 2 | 最多 2 个逻辑驱动器 / up to 2 drives |
| _FS_TINY | 1 | Tiny 模式 (节省 4KB/文件) |
| _USE_LFN | 3 | 堆分配 LFN 缓冲 (255 字符) |
| _FS_REENTRANT | 0 | 非重入 (单任务) |
| _FS_RPATH | 0 | 无相对路径支持 |
| _MIN_SS / _MAX_SS | 512 / 4096 | 扇区大小范围 |

## 已知限制 / Known Limitations

- RAMFS 文件最大 127 字节
- SD→RAMFS 复制超过 127 字节被拒绝
- FatFs 非重入模式 (多任务访问需加锁)
- 编译产物约 160 KB (Flash), 83 KB (RAM)
