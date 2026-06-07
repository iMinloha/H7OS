# H7OS — Embedded Operating System Documentation

Linux-style embedded OS for STM32H743IIT6 (FK743M2-IIT6), built on FreeRTOS.
In-memory filesystem (RAMFS), FAT32 SD card, USB CDC shell, device tree with `file_ops` drivers.

---

## 1. Hardware

| Component  | Detail                                  |
|------------|-----------------------------------------|
| **MCU**    | STM32H743IIT6 (Cortex-M7, 480 MHz)      |
| **SDRAM**  | 32 MB via FMC                           |
| **QSPI**   | W25Qxx 16 MB Flash (persistence)        |
| **Display**| LTDC + DMA2D LCD, JPEG decoder          |
| **SD Card**| SDMMC1, FAT32 via FatFs R0.12c          |
| **USB**    | OTG FS, CDC virtual COM port            |
| **USART**  | USART1 (printf + DMA RX), USART2         |
| **GPIO**   | PF7 (output), PH4/6/7 (output)          |
| **PWM**    | TIM2 CH1 → PA0, CH2 → PA1 @ 50 Hz       |
| **ADC**    | ADC1 CH4 (12-bit), ADC3 (temp sensor)   |
| **I2C**    | I2C1                                    |
| **Timer**  | TIM1 (HAL tick), RTC, RNG               |

---

## 2. Build & Flash

Toolchain: `arm-none-eabi-gcc` (GNU Arm Embedded 10 2021.10), CLion + OpenOCD

```bash
cd cmake-build-debug
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug ..
mingw32-make clean && mingw32-make -j8
```

Outputs: `H7OS.elf`, `H7OS.hex`, `H7OS.bin`

---

## 3. Architecture

### 3.1 Boot Sequence

```
main()
  → HAL_Init()
  → SystemClock_Config()           // HSE 25MHz × PLL → 480 MHz
  → MPU_Config()                   // SDRAM regions at 0xC0000000
  → MX_GPIO_Init()
  → MX_DMA_Init() / MX_MDMA_Init()
  → MX_FMC_Init()                  // SDRAM
  → MX_QUADSPI_Init()             // W25Qxx flash
  → MX_SDMMC1_SD_Init()           // SD card
  → MX_USART1_UART_Init()         // debug UART
  → MX_LTDC_Init() / MX_DMA2D_Init() / MX_JPEG_Init()  // display
  → MX_ADC3_Init() / MX_ADC1_Init()
  → MX_I2C1_Init() / MX_TIM2_Init()
  → MX_RNG_Init() / MX_RTC_Init()
  → MX_FATFS_Init()               // FATFS_LinkDriver → SDPath = "0:/"
  → BSP_SD_Init()                 // SD hardware init
  → Touch_Init()
  → MX_USB_DEVICE_Init()
  → MX_FREERTOS_Init()
      → MemControl_Init()         // TLSF pools over SDRAM (flashSDRAM + create)
      → taskGlobalInit()          // DrTInit + QSPI/W25Qxx + createCPU
      → ThreadInit()              // create all RTOS tasks
  → osKernelStart()
```

### 3.2 RTOS Tasks

| Task          | Priority        | Stack  | Function                                                |
|---------------|-----------------|--------|---------------------------------------------------------|
| `QueueInit`   | Normal          | 4 KB   | SD mount, FS deserialize, device registration, then sleep |
| `ShellTask`   | Normal          | 4 KB   | USB CDC shell: read line → execCMD → print prompt       |
| `TaskManager` | AboveNormal     | 1 KB   | CPU load calculation every 1s via tick accumulation     |
| `xNone`       | Normal          | 512 B  | Idle task, toggles PH7 LED, baseline for CPU load       |
| `xTaskTest`   | Normal          | 2 KB   | Test/placeholder task                                   |

### 3.3 Memory Layout

```
SDRAM (32 MB total, via FMC at 0xC0000000):
┌─────────────────┐ 0xC0000000
│ Video   8 MB    │ LTDC framebuffer
├─────────────────┤ 0xC0800000
│ Kernel 14 MB    │ TLSF pool: DrT nodes, FS structs, command list, task structs
├─────────────────┤ 0xC1600000
│ User   10 MB    │ TLSF pool: RAMFS data, general purpose allocations
└─────────────────┘

QSPI Flash (16 MB):
  Binary FS persistence (save/load with CRC32)
  Format: [magic "H7FS"][CRC32][dirs][files][end "END\0"]
```

### 3.4 Allocator

**TLSF** (Two-Level Segregated Fit) — `Drivers/Kernel/src/tlsf.c`

| Function         | Pool   | Purpose                              |
|------------------|--------|--------------------------------------|
| `kernel_alloc(n)`| Kernel | DrT nodes, FS structs, CMD entries   |
| `kernel_free(p)` | Kernel | Free kernel allocation               |
| `ram_alloc(n)`   | User   | General purpose                      |
| `ram_free(p)`    | User   | Free user allocation                 |

---

## 4. RAMFS — In-Memory Filesystem

### 4.1 Data Structures

```c
/* Directory node — represents a directory in the filesystem tree */
struct FS {
    char       *path;             // directory name (e.g. "dev", "usr")
    DrTNode_t   node;             // first child device in linked list
    int         node_count;       // number of child devices
    Task_t      tasklist;         // task list (only /proc)
    uint8_t     allow_rm;         // 0 = system dir (protected), 1 = user dir
    char       *sd_mount_path;    // SD mount: base path on SD card (NULL = not mounted)
    char       *sd_cd_path;       // SD mount: current sub-path within mount
    FS_t        parent;           // parent directory
    FS_t        child_next;       // first child directory
    FS_t        level_next;       // next sibling directory
};

/* Device / File node — hardware device, file, or mount point */
struct DrTNode {
    void           *device;       // HAL handle (&huart1, &hqspi, &SDFatFS, or gpio_dev_t*)
    DeviceStatus_E  status;       // DEVICE_OFF / ON / SUSPEND / ERROR / BUSY
    DeviceType_E    type;         // see enum below
    char           *name;         // device name (e.g. "CPU", "USART1")
    char           *description;  // human-readable description
    void           *data;         // generic buffer: file content (128B) or driver data
    void           *fops;         // bsp_file_ops_t* for read/write/open/close
    Mutex_t         mutex;        // access mutex
    DrTNode_t       next;         // next sibling device
    FS_t            parent;       // parent directory
};

/* Task / Process node */
struct Task {
    char           *name;         // task name
    TaskStatus_E    status;       // TASK_READY / RUNNING / SUSPEND / STOP
    float           cpu;          // CPU usage percentage
    TaskPriority_E  priority;     // NORMAL / HIGH / ROOT / SYSTEM
    osThreadId      handle;       // FreeRTOS task handle
    uint32_t        lastWakeTime; // tick at last wake
    uint32_t        accumulatedTime; // total ticks spent running
    uint8_t         PID;          // process ID
    Task_t          next;         // next task in linked list
};

/* Shell command entry */
struct CMD {
    char     *name;               // command name (e.g. "ls", "mkdir")
    char     *description;        // help text
    char     *usage;              // usage string
    Comand_t  cmd;                // handler: void (*)(int argc, char **argv)
    CMD_t     next;               // next command in linked list
};
```

### 4.2 Device Types

```c
enum DeviceType {
    DEVICE_TIMER,      // Timer / PWM
    DEVICE_BS,         // Base device (CPU)
    DEVICE_STORAGE,    // Storage (QSPI, SD, eMMC)
    DEVICE_DISPLAY,    // Display (RGB, LVDS, HDMI)
    DEVICE_INPUT,      // Input device
    DEVICE_SERIAL,     // Serial (USART, UART, SPI, I2C, CAN)
    DEVICE_TRANSPORT,  // Transport (USB, ETH, WiFi)
    DEVICE_VOTAGE,     // Voltage / ADC
    DEVICE_TASK,       // Task info (proc)
    FILE_SYSTEM,       // File system mount
    DrTFILE,           // Regular file (created by touch)
    APP,               // Application binary
};

enum DeviceStatus {
    DEVICE_OFF,        // Powered off
    DEVICE_ON,         // Ready (default)
    DEVICE_SUSPEND,    // Suspended
    DEVICE_ERROR,      // Error state
    DEVICE_BUSY,       // In use (after dev_open)
};
```

### 4.3 Filesystem Tree

```
/                                RAM_FS (root, allow_rm=0)
├── dev/                         hardware devices
│   ├── cpu/CPU                  DEVICE_BS      + cpu_fops
│   ├── serial/USART1            DEVICE_SERIAL  + usart1_fops (IRQ ringbuf)
│   ├── gpio/PF7                 DEVICE_SERIAL  + gpio_fops (output)
│   ├── pwm/PWM2_CH1             DEVICE_TIMER   + pwm_fops (50Hz, PA0)
│   │    /PWM2_CH2               DEVICE_TIMER   + pwm_fops (50Hz, PA1)
│   ├── adc/ADC1                 DEVICE_VOTAGE  + adc_fops (12-bit)
│   └── i2c/I2C1                 DEVICE_SERIAL  + i2c_fops (addr<<1)
├── mnt/                         storage mounts
│   ├── QSPI                     DEVICE_STORAGE (W25Qxx handle)
│   └── SDcard                   FILE_SYSTEM    (FATFS handle)
├── usr/                         user data (allow_rm=0, children persisted)
│   └── <user files/dirs>        created by mkdir/touch, saved to QSPI flash
├── proc/                        process table (Task struct linked list)
│   ├── xShell
│   ├── xTaskManager
│   ├── xTaskTest
│   └── xNoneTask
└── bin/                         user applications (reserved)
```

### 4.4 Core API

```c
/* ── Directory operations ── */
FS_t  fs_create_child(FS_t parent, char *path, uint8_t allow_rm);  // create dir
FS_t  getFSChild(FS_t parent, char *path);                          // find child dir
FS_t  loadPath(char *path);            // resolve path → FS node
FS_t  ram_deep_mkdir(char *path);      // mkdir -p (create intermediate dirs)
void  ram_rm(char *path);              // remove dir or file (auto-detect)
void  ram_ls(char *path);              // list dir contents (RAMFS + SD mount)
FS_t  ram_cd(char *path);              // change directory (supports SD navigation)
void  ram_pwd(FS_t fs, char *path);    // get absolute path string
void  ram_touch(char *path, void *data, uint16_t len);  // create file with content

/* ── Device operations ── */
void  addDevice(char *path, void *hw, char *name, char *desc,
                DeviceType_E type, DeviceStatus_E status, void *driver);
DrTNode_t loadDevice(char *path);      // resolve path → DrTNode
int   dev_open(const char *path);      // open device (→ BUSY)
int   dev_close(const char *path);     // close device (→ ON)
int   dev_read(const char *path, uint8_t *buf, uint32_t len);   // read from device
int   dev_write(const char *path, const uint8_t *buf, uint32_t len); // write

/* ── SD mount ── */
char* fs_to_sd_path(const char *rel);  // convert relative → SD card path

/* ── Task management ── */
void   addThread(Task_t task);
Task_t getThread(char *name);
Task_t getThreadByPID(uint8_t pid);
Task_t getTaskList(void);
Task_t getTaskByHandle(osThreadId handle);
Task_t loadTask(char *path);
```

### 4.5 Registration Macros

```c
// Shell command
CMD("name", "description", "usage", handler_func);

// Device with fops
DEV_REGISTER("dev/subdir", hw_ptr, "NAME", "desc", DEVICE_TYPE, &fops);

// GPIO
GPIO_OUTPUT(GPIOF, GPIO_PIN_7, "PF7");    // → /dev/gpio/PF7
GPIO_INPUT(GPIOA,  GPIO_PIN_0, "BTN");    // → /dev/gpio/BTN

// PWM (freq in Hz)
PWM_REGISTER(htim2, TIM_CHANNEL_1, 50, "PWM2_CH1");  // → /dev/pwm/PWM2_CH1 @ 50Hz

// ADC
ADC_REGISTER(hadc1, "ADC1");              // → /dev/adc/ADC1

// I2C
I2C_REGISTER(hi2c1, "I2C1");             // → /dev/i2c/I2C1

// SPI (when HAL module enabled)
SPI_REGISTER(hspi1, "SPI1");             // → /dev/spi/SPI1
```

---

## 5. Filesystem Persistence

### 5.1 Workflow

```
save    → walk RAMFS tree
        → collect all FS directories + DrTFILE nodes with data
        → build binary blob with CRC32
        → chip-erase QSPI flash (~20s)
        → write blob → read-back verify

reboot  → FS_Deserialize()
        → read blob from flash
        → verify CRC32
        → recreate dirs via ram_deep_mkdir
        → recreate files via ram_touch (with content)
        → skip system dirs already created by DrTInit

reset   → chip-erase QSPI (prompts y/n)
flash   → chip-erase QSPI (no confirm)
```

### 5.2 Flash Layout

```
Offset  Size   Field
0       4      Magic "H7FS" (0x53463748)
4       4      CRC32 (over everything after this field)
8       2      Directory count
10      2      File count
12+     N      Directory entries:
                 [2B path_len] [path bytes incl. \0]
        M      File entries:
                 [2B path_len] [path bytes incl. \0]
                 [2B data_len] [data bytes]
end-4   4      End marker "END\0" (0x454E4400)
```

---

## 6. Commands Reference

### `ls` — List directory contents

```
ls               list current directory
ls /dev/gpio     list specific path
```

Shows:
- **Green**: Subdirectories (RAMFS) and SD card directories
- **Blue**: Devices / files
- **Yellow**: SD card files
- **Purple**: Tasks (only in /proc)

### `cd` — Change directory

```
cd /mnt/sd        absolute path
cd subdir         relative path
cd ..             parent (works in SD mounts too)
cd /              root
```

When inside a mount point, `cd subdir` navigates the SD card. The prompt shows `[SD:path]`.

### `pwd` — Print working directory

Shows current RAMFS path + SD sub-path if inside a mount point.

### `mkdir` — Create directory

```
mkdir /usr/data    create in RAMFS
mkdir sub          create relative (RAMFS or SD if in mount)
```

Inside an SD mount point, creates on the SD card via `f_mkdir`.

### `touch` — Create file

```
touch notes.txt              create empty file
touch config.ini hello=world create with content
```

Inside SD mount → creates on SD via `f_open`/`f_write`.
RAMFS files are `DrTFILE` nodes with 128-byte data buffer.

### `cat` — Print file contents

```
cat /usr/readme.txt    print RAMFS file
cat data.log           print SD file (inside mount point)
```

Reads from `DrTNode.data` (RAMFS) or via FatFs `f_read` (SD).

### `rm` — Remove file or directory

```
rm /usr/old         auto-detect: directory → recursive delete, file → unlink
rm test.txt         relative path
```

Inside SD mount → deletes on SD via `f_unlink`.

### `tree` — Filesystem tree

```
tree                default depth
tree 3              max depth 3
```

Colors: blue=devices, purple=tasks, green=directories. Mount points marked `-> [SD]`.

### `mount` — Mount SD to RAMFS

```
mount / /mnt/sd         SD root → /mnt/sd
mount /mnt/sd           single-arg: SD root → /mnt/sd
mount photos /mnt/pics  SD photos/ → /mnt/pics
```

Verifies SD path exists, creates RAMFS dir if needed, sets `FS.sd_mount_path`.

### `use` — Device I/O via fops

```
# GPIO
use /dev/gpio/PF7 write 1       PF7 high
use /dev/gpio/PF7 write 0       PF7 low
use /dev/gpio/PF7 read 1        read state → '1' or '0'

# PWM (50 Hz on PA0/PA1)
use /dev/pwm/PWM2_CH1 write 50  50% duty
use /dev/pwm/PWM2_CH1 write 0   stop
use /dev/pwm/PWM2_CH2 write 75  75% duty

# ADC (12-bit, 0-4095)
use /dev/adc/ADC1 read 10       read raw ADC value

# I2C (addr<<1 protocol: first byte = 7-bit address)
use /dev/i2c/I2C1 write 50 00 41 42    write 0x41,0x42 to dev 0x50 reg 0x00
use /dev/i2c/I2C1 read 50 4            read 4 bytes from dev 0x50

# USART1 (non-blocking IRQ ring buffer)
use /dev/serial/USART1 open            start IRQ receive
use /dev/serial/USART1 write hello     send "hello"
use /dev/serial/USART1 read 20         non-blocking read (returns available bytes)
use /dev/serial/USART1 close           stop IRQ

# CPU info
use /dev/cpu/CPU read 100             show name/freq/temp/load
```

### `info` — Device or task detail

```
info /dev/cpu/CPU      show CPU info (name, frequency, temperature, load)
info xShell            show task info (PID, priority, CPU%, status)
```

### `echo` — Print text

```
echo hello world       print to console
```

### `help` — Command help

```
help                   list all commands
help ls                show ls usage
```

### `save` — Persist filesystem

```
save
→ save: 6 dirs, 1 files → erase OK, writing... verify... OK (156 bytes)
```

### `reboot` — Save and restart

Saves filesystem then triggers software reset.

### `reset` — Factory reset

Prompts `y/n`, then erases QSPI flash. Requires reboot to take effect.

### `flash` — Erase QSPI flash

Erases the entire QSPI flash chip without confirmation.

---

## 7. Device Drivers (file_ops)

### 7.1 Interface

```c
// Drivers/BSP/bsp_file_ops.h
typedef struct bsp_file_ops {
    int (*open) (void *dev);                              // acquire device → BUSY
    int (*close)(void *dev);                              // release → ON
    int (*read) (void *dev, uint8_t *buf, uint32_t len);  // read data
    int (*write)(void *dev, const uint8_t *buf, uint32_t len); // write data
} bsp_file_ops_t;
```

`dev` parameter receives the `DrTNode.device` pointer (HAL handle or driver struct).

### 7.2 USART1 Driver — Non-blocking Serial

```
Drivers/BSP/Components/usart/
```

- IRQ-based receive with 256-byte ring buffer
- `open` enables USART1 IRQ, starts `HAL_UART_Receive_IT`
- `read` returns available bytes immediately (0 if none)
- `write` blocking transmit via `HAL_UART_Transmit`
- `close` aborts IRQ, disables interrupt

### 7.3 GPIO Driver

```
Drivers/BSP/Components/gpio/
```

```c
typedef struct {
    GPIO_TypeDef *port;
    uint16_t      pin;
    uint8_t       is_output;   // 1=output, 0=input
} gpio_dev_t;
```

- Output: `write "1"` → HIGH, `write "0"` → LOW
- Input: `read` → returns '1' or '0'

### 7.4 PWM Driver

```
Drivers/BSP/Components/pwm/
```

```c
typedef struct {
    TIM_HandleTypeDef *htim;
    uint32_t channel;
} pwm_dev_t;
```

- `write "0"-"100"` → sets duty cycle %, starts PWM
- Auto-calculates prescaler/period from TIM2 clock (240 MHz) to achieve target frequency
- `write "0"` → stops PWM

### 7.5 ADC Driver

```
Drivers/BSP/Components/adc/
```

- Polling read via `HAL_ADC_Start` + `HAL_ADC_PollForConversion`
- Returns raw 12-bit value as decimal string (0-4095)
- Auto-calibrates before each read

### 7.6 I2C Driver

```
Drivers/BSP/Components/i2c/
```

- Protocol: first byte = 7-bit address (auto-shifted left by driver)
- `write <addr> <data...>` → `HAL_I2C_Master_Transmit`
- `read <addr> <n>` → `HAL_I2C_Master_Receive`

### 7.7 CPU Driver

```
Drivers/BSP/Components/cpu/
```

- `read` → returns formatted string: name, frequency, temperature (°C), load (%)
- Internal: reads ADC3 temp sensor, calculates load from idle task stats

### 7.8 Adding a New Driver

```
1. mkdir Drivers/BSP/Components/<name>/
2. Create bsp_<name>.h / bsp_<name>.c
3. Implement bsp_file_ops_t <name>_fops
4. Provide void <name>_device_init(void) calling addDevice("dev/<cat>", ...)
5. Add #include and call in Drivers/BSP/bsp_devices.c → devices_init()
6. Add "Drivers/BSP/Components/<name>" to CMakeLists.txt include path
```

---

## 8. Project Structure

```
H7OS/
├── Applications/              RTOS tasks
│   ├── inc/                   TaskHead.h, xShellTask.h, xTaskManager.h, ...
│   └── src/                   xShellTask.c, xTaskInit.c, xTaskManager.c, ...
├── Command/                   Shell commands
│   ├── Register.c/h           Command registration table (CMD() macros)
│   ├── cat/ cd/ echo/ help/ info/ ls/ mkdir/ mount/ pwd/ rm/ touch/ tree/
│   ├── use/ save/ reboot/ reset/ flash/
│   └── dfu/                   (reserved)
├── Core/                      STM32CubeMX HAL
│   ├── Inc/                   main.h, gpio.h, adc.h, tim.h, i2c.h, ...
│   ├── Src/                   main.c, freertos.c, gpio.c, adc.c, ...
│   └── startup/               startup_stm32h743xx.s
├── Drivers/
│   ├── BSP/
│   │   ├── bsp_file_ops.h     public driver interface
│   │   ├── bsp_devices.h/c    device registration center
│   │   └── Components/
│   │       ├── cpu/           CPU info via ADC3 + runtime stats
│   │       ├── usart/         USART1 IRQ ring buffer (non-blocking)
│   │       ├── gpio/          per-pin input/output
│   │       ├── pwm/           TIM PWM with frequency config
│   │       ├── adc/           ADC polling read
│   │       └── i2c/           I2C master (addr<<1)
│   ├── Kernel/                TLSF allocator (tlsf.c), mutex, memory utils
│   ├── CMSIS/                 Cortex-M7 core headers
│   └── STM32H7xx_HAL/         STM32 HAL library
├── FATFS/
│   ├── App/fatfs.c            SDFatFS, SDPath, MX_FATFS_Init
│   └── Target/                ffconf.h, bsp_driver_sd.c, sd_diskio.c
├── Middlewares/
│   ├── FreeRTOS/              FreeRTOS kernel + CMSIS-RTOS v2 wrapper
│   └── Third_Party/FatFs/     ff.c/h (R0.12c), option/syscall.c
├── RAMFS/
│   ├── RAMFS.h                umbrella include
│   └── init.d/
│       ├── Core/              DrT.h, DrT.c (structures + init)
│       ├── FS/                DrT_fs.c (fs_create_child, mkdir, rm, touch, ls, cd, pwd)
│       ├── Dev/               DrT_dev.c (addDevice, loadDevice, dev_read/write/open/close)
│       ├── Cmd/               DrT_cmd.c (addCMD, execCMD, helpCMD)
│       ├── Mnt/               DrT_mnt.c (fs_to_sd_path)
│       ├── Proc/              DrT_proc.c (addThread, getThread, loadTask)
│       ├── Periph/            cpu.h/c, FS_Serial.h/c (CPU monitor + persistence)
│       ├── Algo/              DFS.h/c (tree walk)
│       ├── ASM/               reset.s, sd_boot.s
│       └── Bin/builder/       builder framework (TODO)
├── USB_DEVICE/                USB CDC (virtual COM port)
├── Utilities/JPEG/            JPEG hardware decoder
├── CMakeLists.txt
├── STM32H743IITX_FLASH.ld     linker script
└── README.md
```

---

## 9. CubeMX Notes

After regenerating code from `.ioc`, verify these manual patches:

| File | What to check |
|------|---------------|
| `FATFS/App/fatfs.c` | `char SDPath[4];` — set by `FATFS_LinkDriver` to `"0:/"` automatically |
| `FATFS/Target/ffconf.h` | `_USE_LFN=0`, `_FS_REENTRANT=0`, `_CODE_PAGE=437` |
| `FATFS/Target/sd_diskio.c` | `#define ENABLE_SD_DMA_CACHE_MAINTENANCE 1` |
| `FATFS/src/option/syscall.c` | `ff_memalloc→kernel_alloc`, `ff_memfree→kernel_free` |
| `Core/Src/freertos.c` | USER CODE RTOS_THREADS: `MemControl_Init()`, `taskGlobalInit()`, `ThreadInit()` |
| `Core/Src/main.c` | USER CODE 2: `MX_USB_DEVICE_Init()`, `BSP_SD_Init()`, `Touch_Init()` |
| `Applications/src/xTaskInit.c`| No duplicate `FATFS_LinkDriver` call; `SDFatFS.database` fix |

In CubeMX FATFS settings: Code Page=US English (437), LFN=Disabled, Reentrant=Disabled.

---

## 10. TODO

- [x] TLSF memory allocator (kernel + user pools)
- [x] RAMFS in-memory filesystem
- [x] DrT device tree with subdirectories
- [x] SD card FAT32 via FatFs
- [x] USB CDC shell + CMD() command system
- [x] Binary FS persistence (save/load QSPI with CRC32)
- [x] CPU load monitoring (tick accumulation)
- [x] SD mount + transparent operations (mkdir/touch/rm/cat)
- [x] file_ops driver framework + DEV_REGISTER macro
- [x] USART1 non-blocking (DMA+IRQ ring buffer)
- [x] GPIO driver (per-pin input/output)
- [x] PWM driver (frequency + duty cycle)
- [x] ADC driver (polling with calibration)
- [x] I2C driver (addr<<1 master)
- [x] CPU info driver (freq/temp/load via fops)
- [x] Modular RAMFS tree (Core/FS/Dev/Cmd/Mnt/Proc/Periph/Algo)
- [ ] Long File Name support (needs working unicode.c)
- [ ] SPI driver (HAL module not yet enabled)
- [ ] Network stack (Ethernet / WiFi)
- [ ] GUI subsystem (LVGL or similar)
- [ ] Power management (sleep modes)
- [ ] Thread-safe FATFS (_FS_REENTRANT=1 semaphore issue)
- [ ] Device permission system (root/user access control)
- [ ] Flash wear leveling for persistence

---

## 11. License

Copyright © 2024 Minloha — https://blog.minloha.cn
