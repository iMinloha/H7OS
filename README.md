# H7OS — 嵌入式操作系统文档

> 现已支持Platform，请移步到master分支查看新版本代码

基于 FreeRTOS 的 Linux 风格嵌入式操作系统，运行于 STM32H743IIT6（FK743M2-IIT6 开发板）。
提供内存文件系统（RAMFS）、FAT32/exFAT SD 卡支持、USB CDC 虚拟串口 Shell、设备树（Device Tree）与 `file_ops` 驱动框架。

---

## 一、硬件平台

| 组件 | 详情 |
|------|------|
| **主控** | STM32H743IIT6（Cortex-M7, 480 MHz） |
| **SDRAM** | 32 MB，通过 FMC 接口 |
| **QSPI Flash** | W25Qxx 16 MB，用于文件系统持久化 |
| **显示屏** | LTDC + DMA2D LCD，JPEG 硬件解码器 |
| **SD 卡** | SDMMC1 接口，FAT32 / exFAT，基于 FatFs R0.12c |
| **USB** | OTG FS，CDC 虚拟串口（USB printf/scanf） |
| **串口** | USART1（printf 调试输出 + DMA 接收），USART2 |
| **GPIO** | PF7（LED 输出），PH4/6/7（输出） |
| **PWM** | TIM2 CH1 → PA0，CH2 → PA1，默认 50 Hz |
| **ADC** | ADC1 CH4（12 位外部），ADC3（内部温度传感器） |
| **I2C** | I2C1 |
| **定时器** | TIM1（HAL 时基），RTC，RNG |

---

## 二、编译与烧录

**工具链**：`arm-none-eabi-gcc`（GNU Arm Embedded 10 2021.10），CLion + OpenOCD

```bash
cd cmake-build-debug
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug ..
mingw32-make clean && mingw32-make -j8
```

**生成文件**：`H7OS.elf`、`H7OS.hex`、`H7OS.bin`

**内存占用**（Debug 配置）：

| 区域 | 用量 | 总容量 | 占比 |
|------|------|--------|------|
| FLASH | ~157 KB | 2 MB | 7.5% |
| RAM_D1 | ~83 KB | 512 KB | 16% |
| DTCMRAM | 0 | 128 KB | 0% |

---

## 三、系统架构

### 3.1 启动流程

```
main()
  ├── HAL_Init()                         // HAL 库初始化
  ├── SystemClock_Config()               // HSE 25MHz × PLL → 480 MHz
  ├── MPU_Config()                       // SDRAM 区域 0xC0000000 内存保护
  ├── 外设初始化（GPIO/DMA/MDMA/FMC/QUADSPI/SDMMC/USART/LTDC/DMA2D/JPEG）
  ├── ADC3/ADC1/I2C1/TIM2/RNG/RTC 初始化
  ├── MX_FATFS_Init()                    // FATFS_LinkDriver → SDPath = "0:/"
  ├── BSP_SD_Init()                      // SD 卡硬件初始化
  ├── Touch_Init()                       // 触摸屏初始化
  ├── MX_USB_DEVICE_Init()               // USB CDC 虚拟串口
  ├── MX_FREERTOS_Init()
  │   ├── MemControl_Init()              // TLSF 内存池（SDRAM 上分配 Kernel + User 池）
  │   ├── taskGlobalInit()
  │   │   ├── DrTInit()                  // 创建 RAMFS 根节点 /dev, /mnt, /usr, /proc, /bin
  │   │   ├── devices_init()             // 注册 cpu/usart/gpio/pwm/adc/i2c 设备驱动
  │   │   ├── SD 卡检测                  // 读取 CID/CardInfo，计算容量
  │   │   ├── QSPI Flash 初始化          // W25Qxx 检测
  │   │   └── createCPU()                // 创建 CPU 监控对象
  │   └── ThreadInit()                   // 创建所有 RTOS 任务
  └── osKernelStart()                    // 启动 FreeRTOS 调度器
```

### 3.2 RTOS 任务

| 任务名 | 优先级 | 栈大小 | 功能 |
|--------|--------|--------|------|
| `QueueInit` | Normal | 4 KB | SD 卡挂载、FS 反序列化、设备注册，完成后休眠 |
| `Shell` | Normal | 4 KB | USB CDC Shell：读取命令行 → execCMD → 打印提示符 |
| `TaskMgr` | AboveNormal | 1 KB | 每秒通过 FreeRTOS 运行时统计更新各任务 CPU 占用率 |
| `Test` | Normal | 2 KB | 测试/占位任务 |
| `Kernel` | Normal | 512 B | 空闲任务（原 xNoneTask），LED 闪烁，用于系统负载基准 |

### 3.3 内存布局

```
SDRAM（32 MB，通过 FMC 映射至 0xC0000000）：
┌─────────────────┐ 0xC0000000
│ 显存     8 MB   │ LTDC 帧缓冲
├─────────────────┤ 0xC0800000
│ Kernel  14 MB   │ TLSF 池：DrT 节点、FS 结构体、命令链表、任务结构体
├─────────────────┤ 0xC1600000
│ User    10 MB   │ TLSF 池：RAMFS 文件数据、通用分配
└─────────────────┘

QSPI Flash（16 MB）：
  文件系统二进制持久化（save/load，CRC32 校验）
  布局：[magic "H7FS" 4B][CRC32 4B][目录条目][文件条目][结束标记 "END\0"]
```

### 3.4 内存分配器

**TLSF**（Two-Level Segregated Fit，两级分离适配）— `Drivers/Kernel/src/tlsf.c`

| 函数 | 内存池 | 用途 |
|------|--------|------|
| `kernel_alloc(n)` | Kernel（14 MB） | DrT 节点、FS 结构体、CMD 条目、设备驱动数据 |
| `kernel_free(p)` | Kernel | 释放内核分配 |
| `ram_alloc(n)` | User（10 MB） | RAMFS 文件数据、通用用途 |
| `ram_free(p)` | User | 释放用户分配 |

---

## 四、RAMFS — 内存文件系统

### 4.1 核心数据结构

```c
/* 目录节点 */
struct FS {
    char       *path;             // 目录名（如 "dev", "usr"）
    DrTNode_t   node;             // 第一个子设备（链表头）
    int         node_count;       // 子设备数量
    Task_t      tasklist;         // 任务链表（仅 /proc 使用）
    uint8_t     allow_rm;         // 0 = 系统目录（保护），1 = 用户目录
    char       *sd_mount_path;    // SD 挂载点：SD 卡上的基准路径（NULL = 未挂载）
    char       *sd_cd_path;       // SD 挂载点：当前子路径（cd 导航用）
    FS_t        parent;           // 父目录
    FS_t        child_next;       // 第一个子目录
    FS_t        level_next;       // 下一个兄弟目录
};

/* 设备/文件节点 */
struct DrTNode {
    void           *device;       // HAL 句柄（&huart1, &SDFatFS, gpio_dev_t* 等）
    DeviceStatus_E  status;       // DEVICE_OFF / ON / SUSPEND / ERROR / BUSY
    DeviceType_E    type;         // 设备类型（见下方枚举）
    char           *name;         // 设备名（如 "CPU", "USART1", "PF7"）
    char           *description;  // 描述文本
    void           *data;         // 通用缓冲区：文件内容（128B）或驱动自定义数据
    void           *fops;         // bsp_file_ops_t* 驱动接口指针
    Mutex_t         mutex;        // 访问互斥锁
    DrTNode_t       next;         // 下一个兄弟设备
    FS_t            parent;       // 父目录
};

/* 任务/进程节点 */
struct Task {
    char           *name;         // 任务名称（如 "Shell", "TaskMgr"）
    TaskStatus_E    status;       // TASK_READY / RUNNING / SUSPEND / STOP
    float           cpu;          // CPU 占用百分比
    TaskPriority_E  priority;     // NORMAL / HIGH / ROOT / SYSTEM
    osThreadId      handle;       // FreeRTOS 任务句柄
    uint32_t        lastWakeTime; // 上次唤醒的 tick
    uint32_t        accumulatedTime; // 累计运行 tick
    uint8_t         PID;          // 进程 ID
    Task_t          next;         // 下一个任务（链表）
};

/* Shell 命令条目 */
struct CMD {
    char     *name;               // 命令名（如 "ls", "mkdir"）
    char     *description;        // 帮助文本
    char     *usage;              // 用法字符串
    Comand_t  cmd;                // 处理函数：void (*)(int argc, char **argv)
    CMD_t     next;               // 下一个命令（链表）
};
```

### 4.2 设备类型与状态

```c
enum DeviceType {
    DEVICE_TIMER,      // 定时器 / PWM
    DEVICE_BS,         // 基础设备（CPU）
    DEVICE_STORAGE,    // 存储设备（QSPI, SD, eMMC）
    DEVICE_DISPLAY,    // 显示设备（RGB, LVDS, HDMI）
    DEVICE_INPUT,      // 输入设备
    DEVICE_SERIAL,     // 串行总线（USART, UART, SPI, I2C, CAN）
    DEVICE_TRANSPORT,  // 传输层（USB, ETH, WiFi）
    DEVICE_VOTAGE,     // 电压采集（ADC, DAC）
    DEVICE_TASK,       // 任务信息（proc）
    FILE_SYSTEM,       // 文件系统挂载
    DrTFILE,           // 普通文件（touch 创建）
    APP,               // 可执行文件
};

enum DeviceStatus {
    DEVICE_OFF,        // 关闭
    DEVICE_ON,         // 就绪（默认）
    DEVICE_SUSPEND,    // 挂起
    DEVICE_ERROR,      // 错误
    DEVICE_BUSY,       // 占用中（dev_open 后）
};

enum TaskPriority {
    TASK_PRIORITY_NORMAL,
    TASK_PRIORITY_HIGH,
    TASK_PRIORITY_ROOT,
    TASK_PRIORITY_SYSTEM,
};

enum TaskStatus {
    TASK_READY,
    TASK_RUNNING,
    TASK_SUSPEND,
    TASK_STOP,
};
```

### 4.3 文件系统树

```
/                                    RAM_FS 根节点（allow_rm=0）
├── dev/                             硬件设备目录
│   ├── cpu/
│   │   └── CPU                      DEVICE_BS      + cpu_fops（名称/频率/温度/负载）
│   ├── serial/
│   │   └── USART1                   DEVICE_SERIAL  + usart1_fops（中断环形缓冲区）
│   ├── gpio/
│   │   └── PF7                      DEVICE_SERIAL  + gpio_fops（LED 输出）
│   ├── pwm/
│   │   ├── PWM2_CH1                 DEVICE_TIMER   + pwm_fops（50Hz, PA0）
│   │   └── PWM2_CH2                 DEVICE_TIMER   + pwm_fops（50Hz, PA1）
│   ├── adc/
│   │   └── ADC1                     DEVICE_VOTAGE  + adc_fops（12 位轮询）
│   └── i2c/
│       └── I2C1                     DEVICE_SERIAL  + i2c_fops（addr 左移 1 位）
├── mnt/                             存储挂载目录
│   ├── QSPI                         DEVICE_STORAGE（W25Qxx 句柄）
│   └── SDcard                       FILE_SYSTEM  （FATFS 句柄，FAT32/exFAT）
├── usr/                             用户数据（allow_rm=0，子节点可持久化）
│   └── <用户文件/目录>              由 mkdir/touch 创建，save 后存入 QSPI Flash
├── proc/                            进程表（Task 结构体链表）
│   ├── Shell                        USB CDC 命令行任务
│   ├── TaskMgr                      CPU 负载监控任务
│   ├── Test                         测试任务
│   └── Kernel                       空闲/内核任务
└── bin/                             用户应用程序（预留）
```

### 4.4 核心 API

```c
/* ── 目录操作 ── */
FS_t  fs_create_child(FS_t parent, char *path, uint8_t allow_rm);   // 创建子目录
FS_t  getFSChild(FS_t parent, char *path);                           // 查找子目录
FS_t  loadPath(char *path);              // 解析路径 → FS 节点（支持相对路径）
FS_t  ram_deep_mkdir(char *path);        // mkdir -p（递归创建中间目录）
void  ram_rm(char *path);                // 删除目录或文件（自动检测类型）
void  ram_ls(char *path);                // 列出目录内容（RAMFS + SD 挂载）
FS_t  ram_cd(char *path);                // 切换目录（支持 SD 卡内导航和 cd .. 返回）
void  ram_pwd(FS_t fs, char *path);      // 获取当前绝对路径字符串
void  ram_touch(char *path, void *data, uint16_t len);  // 创建文件并写入内容

/* ── 设备操作 ── */
void  addDevice(char *path, void *hw, char *name, char *desc,
                DeviceType_E type, DeviceStatus_E status, void *driver);  // 支持多级路径
DrTNode_t loadDevice(char *path);         // 解析路径 → DrTNode
int   dev_open(const char *path);         // 打开设备（状态 → BUSY）
int   dev_close(const char *path);        // 关闭设备（状态 → ON）
int   dev_read(const char *path, uint8_t *buf, uint32_t len);   // 读取设备
int   dev_write(const char *path, const uint8_t *buf, uint32_t len);  // 写入设备

/* ── SD 挂载 ── */
char* fs_to_sd_path(const char *rel);     // 将相对路径转换为 SD 卡绝对路径

/* ── 任务管理 ── */
void   addThread(Task_t task);            // 添加任务到 /proc
Task_t getThread(char *name);             // 按名称查找任务
Task_t getThreadByPID(uint8_t pid);       // 按 PID 查找任务
Task_t getTaskList(void);                 // 获取任务链表头
Task_t getTaskByHandle(osThreadId handle);// 按句柄查找任务
Task_t loadTask(char *path);              // 按路径查找任务
```

### 4.5 注册宏

```c
// Shell 命令注册
CMD("name", "description", "usage", handler_func);

// 设备注册（自动绑定 fops）
DEV_REGISTER("dev/subdir", hw_ptr, "NAME", "desc", DEVICE_TYPE, &fops);

// GPIO 引脚
GPIO_OUTPUT(GPIOF, GPIO_PIN_7, "PF7");     // → /dev/gpio/PF7（输出）
GPIO_INPUT(GPIOA, GPIO_PIN_0, "BTN");      // → /dev/gpio/BTN（输入）

// PWM 通道（Hz）
PWM_REGISTER(htim2, TIM_CHANNEL_1, 50, "PWM2_CH1");  // → /dev/pwm/PWM2_CH1 @ 50Hz

// ADC 通道
ADC_REGISTER(hadc1, "ADC1");               // → /dev/adc/ADC1

// I2C 总线
I2C_REGISTER(hi2c1, "I2C1");              // → /dev/i2c/I2C1
```

---

## 五、设备驱动框架（file_ops）

### 5.1 驱动接口

```c
// Drivers/BSP/bsp_file_ops.h
typedef struct bsp_file_ops {
    int (*open) (void *dev);                                // 获取设备 → BUSY
    int (*close)(void *dev);                                // 释放 → ON
    int (*read) (void *dev, uint8_t *buf, uint32_t len);   // 读取数据
    int (*write)(void *dev, const uint8_t *buf, uint32_t len); // 写入数据
} bsp_file_ops_t;
```

`dev` 参数接收 `DrTNode.device` 指针（HAL 句柄或驱动自定义结构体）。所有驱动均通过 `use` 命令交互。

### 5.2 USART1 驱动 — 非阻塞串口

```
Drivers/BSP/Components/usart/
```

- 基于中断接收，256 字节环形缓冲区
- `open`：使能 USART1 中断，启动 `HAL_UART_Receive_IT`
- `read`：立即返回可用字节数（无数据时返回 0，不阻塞）
- `write`：阻塞发送，通过 `HAL_UART_Transmit`
- `close`：中止中断接收，关闭中断

### 5.3 GPIO 驱动

```
Drivers/BSP/Components/gpio/
```

```c
typedef struct {
    GPIO_TypeDef *port;
    uint16_t      pin;
    uint8_t       is_output;   // 1 = 输出模式，0 = 输入模式
} gpio_dev_t;
```

- 输出模式：`write "1"` → 高电平，`write "0"` → 低电平
- 输入模式：`read` → 返回 `'1'` 或 `'0'`

### 5.4 PWM 驱动

```
Drivers/BSP/Components/pwm/
```

```c
typedef struct {
    TIM_HandleTypeDef *htim;
    uint32_t channel;
} pwm_dev_t;
```

- `write "0" ~ "100"` → 设置占空比百分比，自动启停 PWM
- 自动根据目标频率计算预分频器/周期（TIM2 时钟 240 MHz）
- `write "0"` → 停止 PWM 输出

### 5.5 ADC 驱动

```
Drivers/BSP/Components/adc/
```

- 轮询读取，`HAL_ADC_Start` + `HAL_ADC_PollForConversion`
- 每次读取前自动校准（`HAL_ADCEx_Calibration_Start`）
- 返回原始 12 位数值字符串（0 ~ 4095）

### 5.6 I2C 驱动

```
Drivers/BSP/Components/i2c/
```

- 协议：第一个字节 = 7 位设备地址（驱动内部自动左移 1 位）
- `write <addr> <data...>` → `HAL_I2C_Master_Transmit`
- `read <addr> <n>` → `HAL_I2C_Master_Receive`

### 5.7 CPU 驱动

```
Drivers/BSP/Components/cpu/
```

- `read` → 返回格式化字符串：名称、频率、温度（°C）、负载（%）
- 内部：读取 ADC3 内部温度传感器计算温度，通过 FreeRTOS DWT 周期计数器获取精确 CPU 负载

### 5.8 添加新驱动

```
1. 创建 Drivers/BSP/Components/<name>/ 目录
2. 新建 bsp_<name>.h 和 bsp_<name>.c
3. 实现 bsp_file_ops_t <name>_fops 结构体
4. 实现 void <name>_device_init(void) 函数，调用 addDevice("dev/<cat>", ...)
5. 在 Drivers/BSP/bsp_devices.c 中 #include 并调用 device_init
6. 若需新目录（如 spi），在 DrT.c 的 DrTInit() 中添加 fs_create_child(dev, "spi", 0)
```

---

## 六、文件系统持久化

### 6.1 工作流程

```
save    → 遍历 RAMFS 树
        → 收集所有 FS 目录 + DrTFILE 节点（含数据，最大 127 字节）
        → 构建二进制数据块，计算 CRC32 校验
        → 整片擦除 QSPI Flash（约 20 秒）
        → 写入数据块 → 读回验证

reboot  → FS_Deserialize() 启动时调用
        → 从 Flash 读取数据块
        → CRC32 验证
        → 通过 ram_deep_mkdir 重建目录
        → 通过 ram_touch 重建文件（含内容）
        → 跳过 DrTInit 已创建的系统目录（dev/mnt/usr/proc/bin）

reset   → 整片擦除 QSPI（确认 y/n）
flash   → 整片擦除 QSPI（无确认）
```

### 6.2 Flash 数据布局

```
偏移    大小    字段
0       4       Magic "H7FS"（0x53463748，小端）
4       4       CRC32（校验从偏移 8 开始的所有数据）
8       2       目录条目数量
10      2       文件条目数量
12+     N       目录条目：[2B 路径长度] [路径字节含 \0]
        M       文件条目：[2B 路径长度] [路径字节含 \0] [2B 数据长度] [数据字节]
末尾-4  4       结束标记 "END\0"（0x454E4400）
```

---

## 七、Shell 命令参考

### 通用约定

- 提示符格式：`root:/当前路径$`
- SD 卡挂载点内显示：`root:/mnt/sd [SD:子路径]$`
- 颜色编码：
  - 🟢 绿色：RAMFS 子目录、SD 卡目录
  - 🔵 蓝色：设备 / 文件节点
  - 🟡 黄色：SD 卡文件
  - 🟣 紫色：任务/进程
  - 🔴 红色：错误信息

### `ls` — 列出目录内容

```
ls                列出当前目录
ls /dev/gpio      列出指定路径
```

显示当前目录下的子目录、设备、文件、任务以及 SD 卡内容（若已挂载）。

### `cd` — 切换目录

```
cd /mnt/sd        绝对路径
cd subdir         相对路径（支持 RAMFS 和 SD 卡内导航）
cd ..             返回上级目录（支持从 SD 挂载点返回 RAMFS）
cd /              返回根目录
```

进入 SD 卡挂载点后，提示符自动显示 `[SD:]` 标记及当前子路径。

### `pwd` — 显示当前路径

显示当前 RAMFS 绝对路径，若在 SD 卡挂载点内则同时显示 SD 子路径。

### `mkdir` — 创建目录

```
mkdir /usr/data    在 RAMFS 中创建
mkdir sub          相对路径创建（RAMFS 或 SD 卡内）
```

在 SD 卡挂载点内时，通过 `f_mkdir` 在 SD 卡上创建。

### `touch` — 创建文件

```
touch notes.txt              创建空文件
touch config.ini hello=world 创建文件并写入内容
```

- RAMFS 中：创建 `DrTFILE` 节点，128 字节数据缓冲区
- SD 卡内：通过 `f_open` + `f_write` 创建实际文件

### `cat` — 输出文件内容

```
cat /usr/readme.txt  输出 RAMFS 文件内容
cat data.log         输出 SD 卡文件内容
```

- RAMFS：读取 `DrTNode.data` 缓冲区
- SD 卡：通过 `f_read` 读取，输出后自动追加换行

### `rm` — 删除文件或目录

```
rm /usr/old          自动检测类型（目录递归删除 / 文件直接删除）
rm test.txt          相对路径删除
```

- SD 卡内：通过 `f_unlink` 删除
- 成功时静默（Unix 风格），仅在失败时报错

### `tree` — 文件系统树

```
tree                默认深度
tree 3              最大深度 3
```

显示目录树结构，包括：
- RAMFS 子目录（绿色）、设备（蓝色）、任务（紫色）
- SD 卡挂载点下的目录（绿色带 `/`）和文件（黄色）
- 挂载点标记为 `-> [SD]`

### `mount` — 挂载 SD 卡到 RAMFS

```
mount /mnt/sd           将 SD 卡根目录挂载到 /mnt/sd
mount / /mnt/sd         同上（显式指定 SD 根路径）
mount photos /mnt/pics  将 SD 卡 photos/ 目录挂载到 /mnt/pics
```

- 自动验证 SD 路径存在
- 自动创建 RAMFS 目标目录（若不存在）
- 支持 FAT32 和 exFAT 格式

### `info` — 查看设备或任务详情

```
info /dev/cpu/CPU      查看 CPU 信息（名称、频率、温度、负载）
info Shell             查看任务信息（PID、优先级、CPU 占用、状态）
info /dev/gpio/PF7     查看设备信息（名称、描述、类型、状态）
```

任务信息表格格式：

```
Task             PID   Priority     Load     Status
-------------------------------------------------------------
TaskMgr          1     System       12.5%    Running
Shell            2     System        3.2%    Ready
```

### `use` — 设备 I/O 操作

```
# GPIO
use /dev/gpio/PF7 write 1        PF7 输出高电平
use /dev/gpio/PF7 write 0        PF7 输出低电平
use /dev/gpio/PF7 read 1         读取引脚状态 → '1' 或 '0'

# PWM（50 Hz on PA0/PA1）
use /dev/pwm/PWM2_CH1 write 50   50% 占空比
use /dev/pwm/PWM2_CH1 write 0    停止输出
use /dev/pwm/PWM2_CH2 write 75   75% 占空比

# ADC（12 位，0 ~ 4095）
use /dev/adc/ADC1 read 10        读取 ADC 原始值

# I2C（首字节 = 7 位地址）
use /dev/i2c/I2C1 write 50 00 41 42    向 0x50 设备寄存器 0x00 写入 0x41, 0x42
use /dev/i2c/I2C1 read 50 4            从 0x50 设备读取 4 字节

# USART1（非阻塞中断环形缓冲区）
use /dev/serial/USART1 open            启动中断接收
use /dev/serial/USART1 write hello     发送 "hello"
use /dev/serial/USART1 read 20         非阻塞读取（返回当前可用字节）
use /dev/serial/USART1 close           停止中断接收

# CPU 信息
use /dev/cpu/CPU read 100              获取 CPU 名称/频率/温度/负载
```

### `echo` — 输出文本

```
echo hello world       输出到控制台
```

### `help` — 帮助信息

```
help                   列出所有可用命令
help ls                查看 ls 命令用法
```

### `save` — 持久化文件系统

```
save
→ save: 6 dirs, 1 files → erase OK, writing... verify... OK (156 bytes)
```

将当前 RAMFS 用户文件系统保存到 QSPI Flash，下次启动自动恢复。

### `reboot` — 保存并重启

保存文件系统后触发软件复位。

### `reset` — 恢复出厂设置

提示确认（y/n），擦除 QSPI Flash。需重启生效。

### `flash` — 擦除 QSPI Flash

无需确认，直接擦除整个 QSPI Flash 芯片。

---

## 八、CPU 负载监控

系统使用 **Cortex-M7 DWT（Data Watchpoint and Trace）周期计数器** 实现精确的 CPU 负载统计：

1. **硬件定时器**：DWT 周期计数器以 CPU 主频（480 MHz）运行，提供 32 位高精度计时
2. **FreeRTOS 运行时统计**：每次任务切换时，FreeRTOS 自动累加当前任务的执行时间（不包含阻塞/睡眠时间）
3. **TaskMgr 任务**：每秒调用 `uxTaskGetSystemState()` 获取每个任务的累计运行时间
4. **CPU 占用计算**：`任务 CPU% = 任务运行时间 / 总运行时间 × 100`
5. **系统负载**：`系统负载 = 100% - 空闲任务 CPU%`

---

## 九、FATFS 配置

| 选项 | 值 | 说明 |
|------|-----|------|
| `_FS_EXFAT` | 1 | 支持 exFAT 格式（大容量 SD 卡 ≥32GB） |
| `_CODE_PAGE` | 437 | U.S. OEM 代码页 |
| `_USE_LFN` | 3 | 启用长文件名（堆分配缓冲区），区分大小写 |
| `_MAX_LFN` | 255 | 最大文件名长度 |
| `_MAX_SS` | 4096 | 最大扇区大小（exFAT 需要 ≥4096） |
| `_MIN_SS` | 512 | 最小扇区大小 |
| `_VOLUMES` | 2 | 逻辑驱动器数量 |
| `_FS_REENTRANT` | 0 | 关闭内部锁（应用层单任务顺序访问） |
| `_FS_READONLY` | 0 | 读写模式 |
| `_USE_MKFS` | 1 | 启用格式化功能 |
| `_USE_STRFUNC` | 2 | 启用字符串函数（带 LF-CRLF 转换） |

---

## 十、项目结构

```
H7OS/
├── Applications/                  RTOS 任务
│   ├── inc/                       TaskHead.h, xShellTask.h, xTaskManager.h, ...
│   └── src/                       xShellTask.c, xTaskInit.c, xTaskManager.c, test.c
├── Command/                       Shell 命令
│   ├── Register.c/h               命令注册表（CMD() 宏）
│   ├── cat/ cd/ echo/ help/ info/ ls/ mkdir/ mount/ pwd/ rm/ touch/ tree/
│   ├── use/ save/ reboot/ reset/ flash/
│   └── dfu/                       （预留）
├── Core/                          STM32CubeMX HAL 层
│   ├── Inc/                       main.h, FreeRTOSConfig.h, gpio.h, adc.h, tim.h, i2c.h, ...
│   ├── Src/                       main.c, freertos.c（含 DWT 运行时统计）, 各外设 init
│   └── Startup/                   startup_stm32h743iitx.s
├── Drivers/
│   ├── BSP/
│   │   ├── bsp_file_ops.h         驱动公共接口
│   │   ├── bsp_devices.h/c        设备注册中心（devices_init）
│   │   └── Components/
│   │       ├── cpu/               CPU 信息（ADC3 温度 + 运行时统计）
│   │       ├── usart/             USART1 中断环形缓冲区（非阻塞）
│   │       ├── gpio/              引脚级输入/输出
│   │       ├── pwm/               TIM PWM（可配置频率）
│   │       ├── adc/               ADC 轮询读取（自动校准）
│   │       └── i2c/               I2C 主机（地址左移 1 位）
│   ├── Kernel/                    TLSF 分配器（tlsf.c），互斥锁，内存工具
│   ├── CMSIS/                     Cortex-M7 核心头文件
│   └── STM32H7xx_HAL/             STM32 HAL 库
├── FATFS/
│   ├── App/fatfs.c                SDFatFS 全局对象，SDPath，MX_FATFS_Init
│   └── Target/                    ffconf.h（FatFs 配置），sd_diskio.c（SD 磁盘 I/O）
├── Middlewares/
│   ├── FreeRTOS/                  FreeRTOS 内核 + CMSIS-RTOS v1 封装
│   └── Third_Party/FatFs/         ff.c/h（R0.12c），option/syscall.c（内存管理），
│                                  option/unicode.c（LFN Unicode 转换）
├── RAMFS/
│   ├── RAMFS.h                    统一包含头
│   └── init.d/
│       ├── Core/                  DrT.h, DrT.c（结构体定义 + 系统初始化）
│       ├── FS/                    DrT_fs.c（目录操作、ls/cd/pwd、SD 导航、touch/mkdir/rm）
│       ├── Dev/                   DrT_dev.c（设备注册、设备 I/O 封装）
│       ├── Cmd/                   DrT_cmd.c（命令注册、命令解析执行）
│       ├── Mnt/                   DrT_mnt.c（SD 挂载路径转换）
│       ├── Proc/                  DrT_proc.c（任务/进程管理）
│       ├── Periph/                cpu.h/c（CPU 监控），FS_Serial.h/c（持久化序列化/反序列化）
│       ├── Algo/                  DFS.h/c（tree 目录遍历）
│       └── ASM/                   reset.s（软件复位）
├── USB_DEVICE/                    USB CDC 虚拟串口
├── Utilities/JPEG/                JPEG 硬件解码器
├── CMakeLists.txt                 构建配置
├── STM32H743IITX_FLASH.ld         链接脚本（Flash 运行）
├── STM32H743IITX_RAM.ld           链接脚本（RAM 调试）
├── H7OS.ioc                       CubeMX 工程文件
└── README.md                      本文档
```

---

## 十一、CubeMX 注意事项

重新生成代码后，需确认以下手动修改未被覆盖：

| 文件 | 检查项 |
|------|--------|
| `FATFS/Target/ffconf.h` | `_FS_EXFAT=1`, `_USE_LFN=3`, `_CODE_PAGE=437`, `_MAX_SS=4096`, `_FS_REENTRANT=0` |
| `FATFS/Target/sd_diskio.c` | `#define ENABLE_SD_DMA_CACHE_MAINTENANCE 1` |
| `FATFS/App/fatfs.c` | `char SDPath[4]` — 由 `FATFS_LinkDriver` 自动设为 `"0:/"` |
| `Middlewares/Third_Party/FatFs/src/option/unicode.c` | 需手动添加（CubeMX 不生成），实现 `ff_convert` / `ff_wtoupper` |
| `Core/Src/freertos.c` | USER CODE: `configureTimerForRunTimeStats` + `getRunTimeCounterValue` 使用 DWT 周期计数器 |
| `Core/Src/freertos.c` | USER CODE RTOS_THREADS: `MemControl_Init()`, `taskGlobalInit()`, `ThreadInit()` |
| `Core/Src/main.c` | USER CODE 2: `MX_USB_DEVICE_Init()`, `BSP_SD_Init()`, `Touch_Init()` |
| `Core/Inc/FreeRTOSConfig.h` | `configGENERATE_RUN_TIME_STATS=1`, `configUSE_STATS_FORMATTING_FUNCTIONS=1` |
| `Applications/src/xTaskInit.c` | 不重复调用 `FATFS_LinkDriver`；保留 `SDFatFS.database` 修复代码 |
| `RAMFS/init.d/Dev/DrT_dev.c` | `addDevice()` 支持多级路径（按 `/` 拆分遍历） |
| `RAMFS/init.d/FS/DrT_fs.c` | `loadPath()` 相对路径从 `currentFS` 开始；`ram_cd()` 支持 SD 返回 RAMFS；SD 路径添加驱动前缀 |

---

## 十二、开发日志

### 已完成

- [x] TLSF 两级分离适配内存分配器（Kernel + User 双池）
- [x] RAMFS 内存文件系统（目录/设备/文件/任务四类节点）
- [x] DrT 设备树（dev/mnt/usr/proc/bin 五系统目录）
- [x] SD 卡 FAT32/exFAT 文件系统支持
- [x] FATFS LFN 长文件名（区分大小写，堆分配缓冲区）
- [x] USB CDC 虚拟串口 Shell + CMD() 命令注册系统
- [x] 二进制文件系统持久化（save/load QSPI Flash，CRC32 校验）
- [x] CPU 精确负载监控（DWT 周期计数器 + FreeRTOS 运行时统计）
- [x] SD 卡挂载 + 透明文件操作（ls/cd/mkdir/touch/cat/rm/tree 支持 SD）
- [x] file_ops 驱动框架 + DEV_REGISTER 一键注册宏
- [x] USART1 非阻塞驱动（中断环形缓冲区）
- [x] GPIO 引脚级驱动（输入/输出）
- [x] PWM 驱动（频率 + 占空比）
- [x] ADC 驱动（轮询 + 自动校准）
- [x] I2C 驱动（主机模式）
- [x] CPU 信息驱动（频率/温度/负载）
- [x] 模块化 RAMFS 代码结构（Core/FS/Dev/Cmd/Mnt/Proc/Periph/Algo）
- [x] info 命令表格对齐（固定列宽格式化）
- [x] cat 命令输出后自动换行
- [x] rm 命令静默成功（Unix 风格）
- [x] tree 命令显示 SD 卡文件和目录
- [x] cd .. 从 SD 卡挂载点返回 RAMFS
- [x] loadPath 支持相对路径
- [x] addDevice 支持多级设备路径
- [x] exFAT 大容量 SD 卡兼容（112GB 卡测试通过）

### 待完成

- [ ] 完整 Unicode 转换表（CP437 非 ASCII 字符映射）
- [ ] SPI 驱动（HAL 模块待启用）
- [ ] 网络协议栈（Ethernet / WiFi）
- [ ] GUI 子系统（LVGL 或类似框架）
- [ ] 电源管理（睡眠模式）
- [ ] 设备权限系统（root/user 访问控制）
- [ ] Flash 磨损均衡（持久化存储优化）
- [ ] ELF 加载器 + 应用程序框架（/bin 目录）
- [ ] 文件描述符系统（全局 fd 表，统一 open/read/write/close）

---

## 十三、许可证

Copyright © 2024–2025 Minloha — https://blog.minloha.cn
