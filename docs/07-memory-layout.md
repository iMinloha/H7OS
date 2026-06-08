# 内存布局 / Memory Layout

## FK743M2-IIT6

| 区域 / Region | 起始地址 / Base | 大小 / Size | 用途 / Purpose |
|---|---|---|---|
| ITCM RAM | 0x00000000 | 64 KB | 指令紧耦合内存 / instruction TCM |
| FLASH | 0x08000000 | 2 MB | 程序存储 / program storage |
| DTCM RAM | 0x20000000 | 128 KB | 数据紧耦合内存 / data TCM |
| RAM_D1 | 0x24000000 | 512 KB | 主 SRAM (.data, .bss, heap, stack) |
| RAM_D2 | 0x30000000 | 288 KB | 外设数据 / peripheral data |
| RAM_D3 | 0x38000000 | 64 KB | 低功耗域 / low-power domain |
| **SDRAM** | **0xC0000000** | **32 MB** | FMC 外部 SDRAM |
| QSPI Flash | 0x90000000 | 16 MB | W25Qxx NOR Flash |

## SDRAM 分区 / SDRAM Partitions (32 MB)

| 分区 / Partition | 偏移 / Offset | 大小 / Size | 用途 / Purpose |
|---|---|---|---|
| Video Mem | 0x000000 | 8 MB | LTDC 帧缓冲 (480×272 RGB565) |
| Kernel TLSF | 0x800000 | 14 MB | 内核内存池 (DrT 节点, 文件, 设备) |
| User TLSF | 0x1600000 | 10 MB | 用户内存池 (kernel_alloc/free) |

## FreeRTOS 堆 / FreeRTOS Heap

| 配置 / Config | 值 / Value |
|---|---|
| configTOTAL_HEAP_SIZE | ~30 KB |
| 位置 / Location | RAM_D1 (linker heap) |
| 分配器 / Allocator | FreeRTOS heap_4 (best-fit) |

## TLSF 分配器 / TLSF Allocator

Two-Level Segregated Fit, O(1) 分配/释放。

| API | 池 / Pool | 说明 |
|---|---|---|
| `kernel_alloc(size)` | Kernel (14 MB) | 内核对象分配 |
| `kernel_free(ptr)` | Kernel | 内核对象释放 |
| `ram_alloc(size)` | User (10 MB) | 用户数据分配 |
| `ram_free(ptr)` | User | 用户数据释放 |

## 链接脚本 / Linker Script

- **Flash 执行**: `HAL/<board>/STM32H743IITX_FLASH.ld`
- **RAM 调试**: `HAL/<board>/STM32H743IITX_RAM.ld`

```
FLASH:  0x08000000 (2 MB)   ← .text, .rodata, .init_array
DTCM:   0x20000000 (128 KB) ← 未使用
RAM_D1: 0x24000000 (512 KB) ← .data, .bss, heap (16 KB), stack (16 KB)
RAM_D2: 0x30000000 (288 KB) ← 未使用
RAM_D3: 0x38000000 (64 KB)  ← 未使用
ITCM:   0x00000000 (64 KB)  ← 未使用
```
