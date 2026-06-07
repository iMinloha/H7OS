# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

H7OS is a Linux-like operating system running on an STM32H743IIT6 (Cortex-M7, FK743M2-IIT6 board) built on FreeRTOS. It provides a Unix-style shell interface over USB CDC, an in-memory filesystem (RAMFS), SD card support via FATFS, and a device tree (DrT) inspired by Linux.

## Build & Flash

- **Toolchain**: `arm-none-eabi-gcc` (GNU Arm Embedded Toolchain 10 2021.10)
- **Build system**: CMake 4.0+ with Ninja generator
- **IDE**: CLion (primary); also STM32CubeMX-generated (`.ioc` file)
- **Debug probe**: OpenOCD

```bash
# Configure and build (from cmake-build-debug directory)
cd cmake-build-debug
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug ..
ninja

# Or let CLion handle the build automatically
```

Build outputs in `cmake-build-debug/`: `H7OS.elf`, `H7OS.hex`, `H7OS.bin`.

**Important build notes**:
- The MCU is `STM32H743xx` with hardware FPU (`-mfloat-abi=hard -mfpu=fpv4-sp-d16`).
- Memory layout is defined in `STM32H743IITX_FLASH.ld`.
- SDRAM is configured via FMC at base address `0xC0000000` (32MB) and `0x24000000` (512KB AXI SRAM) — MPU regions are set in `main.c`.

## High-Level Architecture

### Boot Sequence (`Core/Src/main.c` → `Core/Src/freertos.c`)

1. `main()`: HAL init → clock config → peripheral init (`MX_*_Init`) → `MX_FREERTOS_Init()` → `osKernelStart()`
2. `MX_FREERTOS_Init()`: idle task → `MemControl_Init()` (TLSF allocator over SDRAM) → `taskGlobalInit()` (DrT + peripherals) → `ThreadInit()` (creates RTOS threads) → scheduler starts

### Task Layout

| Task | Priority | Purpose |
|------|----------|---------|
| `xTaskManager` | AboveNormal | CPU load monitoring (1 Hz), maintains task linked list |
| `xShell` | Normal | USB CDC shell: reads commands, dispatches via `execCMD()`, prints prompt |
| `QueueInit` (xTaskInit) | Normal | One-shot: mounts SD card FATFS, initializes QSPI flash, then suspends permanently |
| `xNone` (idle) | Normal | FreeRTOS idle task, also used as baseline for CPU load calc |

### Memory Architecture (`Drivers/Kernel/`)

- **TLSF** (Two-Level Segregated Fit) allocator manages SDRAM via FMC.
- Two independent pools carved from SDRAM:
  - **Kernel pool** (14MB): `kernel_alloc()` / `kernel_free()` — used for DrT nodes, FS structures, command nodes, task structs.
  - **User/RAM pool** (10MB): `ram_alloc()` / `ram_free()` — general-purpose, including the RAMFS backing store.
  - **Video pool** (8MB): reserved for LTDC framebuffer.
- `MemControl_Init()` in `Drivers/Kernel/src/memctl.c` sets all this up.
- QSPI flash (W25Qxx, 16MB) provides persistent storage via `flashRead()`/`flashWrite()`.

### RAMFS & Device Tree (DrT) (`RAMFS/`)

The RAM filesystem is a tree of `FS` (directory) and `DrTNode` (device/file) structs, all kernel-allocated. The root `/` contains five system directories:

- `/dev` — devices (CPU, USART1, etc. registered via `addDevice()`)
- `/mnt` — mount points / storage devices (SD card FATFS, QSPI flash)
- `/usr` — user data (persisted to QSPI flash by `save` command)
- `/proc` — process/task list (linked list of `Task_t` structs)
- `/bin` — user applications (builder framework placeholder)

Key files: `RAMFS/init.d/DrT/DrT.c` (filesystem ops + command system), `RAMFS/init.d/DrT/DrT.h` (all type definitions).

### Command System

Commands are registered via the `CMD()` macro defined in `DrT.h`:

```c
CMD("ls", "List files", "ls -path or ls", ls_main);
```

- Registration happens in `Command/Register.c` → `register_main()`, called from `DrTInit()`.
- At runtime, `execCMD()` tokenizes input, looks up the command name in a linked list (`CMDList`), and calls the handler with `(int argc, char **argv)`.
- Each command lives in its own directory under `Command/<name>/` with `<name>_main.c`/`<name>_main.h`.
- Output goes through `USB_printf()` / `USB_color_printf()` (USB CDC virtual COM port).
- Input arrives via `USB_scanf()` (blocking read from CDC).

### Persistence (Context Save — CS)

`RAMFS/init.d/CPU/CS.h` / `CS.c` implements a command-history save/restore system:
- `CS_push()` records each executed command into a linked list.
- `CS_save()` writes the entire list to QSPI flash (with page marker `0xbb`).
- `CS_load()` reads back saved commands on boot and replays them (`CS_Run()`).
- This is how the OS "remembers" state across reboots — commands like `mkdir`, `mount`, etc. are replayed.

### USB CDC Interface (`USB_DEVICE/App/usbd_cdc_if.c`)

- `USB_printf()` — formatted printing over USB CDC (virtual COM port).
- `USB_color_printf(color, fmt, ...)` — wraps text in ANSI/VT100 color codes.
- `USB_scanf(uint8_t *buf)` — blocking line read from USB CDC.
- printf is also retargeted to USART1 (`Core/Src/usart.c`) for hardware UART output.

### Key Peripherals (STM32CubeMX-managed in `Core/`)

- **FMC**: SDRAM (for RAMFS + allocator pools) + LCD interface
- **QSPI**: W25Qxx 16MB flash (persistence layer)
- **SDMMC1**: SD card (FATFS via `FATFS/`)
- **LTDC + DMA2D**: LCD display with JPEG decoder (`Utilities/JPEG/`)
- **USB OTG FS**: CDC virtual COM port
- **USART1**: Debug UART with retargeted printf
- **ADC3**: CPU load current sensing

## Adding a New Command

1. Create `Command/<name>/<name>_main.c` and `<name>_main.h` with a function `void <name>_main(int argc, char **argv)`.
2. Include the header in `Command/Register.c`.
3. Add `CMD("name", "description", "usage", <name>_main);` inside `register_main()`.
4. If the command needs to persist across reboots, push to CS in the handler.
