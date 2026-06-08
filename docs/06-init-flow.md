# 启动流程 / Initialization Flow

## 完整启动序列 / Full Boot Sequence

```
Reset / 复位
  │
  ▼
startup_stm32h743iitx.s        ← 向量表, 栈初始化, 跳转 main
  │                             ← Vector table, stack init, jump to main
  ▼
main.c: main()
  ├─ 1. SCB_EnableICache()             ← 指令缓存 / I-Cache
  ├─ 2. SCB_EnableDCache()             ← 数据缓存 / D-Cache
  ├─ 3. HAL_Init()                     ← HAL 库, SysTick
  │
  ├─ 4. HAL_BoardInit()                ← HAL 层 (hal_init.c)
  │      ├─ MPU_Config()               ←   配置 SDRAM Cacheable
  │      ├─ SystemClock_Config()        ←   HSE 25MHz → PLL1 → 480MHz
  │      ├─ PeriphCommonClock_Config()  ←   PLL2 → ADC 80MHz
  │      ├─ MX_GPIO_Init()             ←   所有外设硬件初始化
  │      ├─ MX_DMA_Init()              ←   All peripheral hardware init
  │      ├─ MX_MDMA_Init()             ←   (MDMA must be before SDMMC)
  │      ├─ MX_FMC_Init()              ←   SDRAM 初始化
  │      ├─ MX_DMA2D_Init()
  │      ├─ MX_QUADSPI_Init()          ←   QSPI Flash
  │      ├─ MX_JPEG_Init()
  │      ├─ MX_SDMMC1_SD_Init()        ←   SDMMC 外设
  │      ├─ MX_USART1_UART_Init()
  │      ├─ MX_RNG_Init()
  │      ├─ MX_RTC_Init()
  │      ├─ MX_FATFS_Init()            ←   FatFs 链接 SD 驱动
  │      ├─ MX_ADC3_Init()
  │      ├─ MX_LTDC_Init()             ←   LCD 显示
  │      ├─ MX_ADC1_Init()
  │      ├─ MX_TIM2_Init()
  │      ├─ MX_USART2_UART_Init()
  │      └─ MX_I2C1_Init()
  │
  ├─ 5. Platform_Init()                ← Platform 层 / Platform layer
  │      └─ board_periph_init()        ←   板级外设 / board-level peripherals
  │           ├─ DFU 检测 (RTC 备份寄存器) / DFU check (RTC backup reg)
  │           ├─ MX_USB_DEVICE_Init()  ←   USB CDC
  │           ├─ BSP_SD_Init()         ←   SD 卡硬件 / SD card hardware
  │           └─ Touch_Init()          ←   触摸屏 / touch screen
  │
  ├─ 6. MX_FREERTOS_Init()            ← FreeRTOS + OS 任务
  │      ├─ osThreadCreate(xNone)      ←   空闲任务 / idle task
  │      ├─ MemControl_Init()          ←   TLSF 分配器 (SDRAM)
  │      ├─ taskGlobalInit()           ←   全局初始化 / global init
  │      │    ├─ DrTInit()             ←     DrT 设备树 / device tree
  │      │    │    └─ Platform_DevicesInit() → 注册 BSP 设备到 /dev
  │      │    │                           Register BSP devices to /dev
  │      │    └─ SD 卡检测 (Platform_SD_GetInfo)
  │      │       + QSPI Flash 初始化 (Platform_QSPI_Init)
  │      └─ ThreadInit()               ←   创建 OS 任务 / create OS tasks
  │           ├─ xTaskInit  → QueueInit
  │           ├─ xShell     → ShellTask
  │           ├─ xTaskManager → TaskManager
  │           └─ xTest → testFunc
  │
  └─ 7. osKernelStart()               ← FreeRTOS 调度器启动
        │                                 Scheduler start
        ├─ QueueInit 任务 (一次性 / one-shot):
        │    ├─ FS_Deserialize()       ←   从 QSPI 还原 RAMFS
        │    │                              Restore RAMFS from QSPI
        │    ├─ f_mount(SD)            ←   挂载 SD 卡 / Mount SD card
        │    ├─ f_mkfs(SD) if needed   ←   如需要, 格式化 SD / Format SD if needed
        │    └─ 挂起 (osDelay 循环) / Suspend (osDelay loop)
        │
        └─ ShellTask 任务:
             ├─ 等待 USB CDC 连接 / Wait for USB CDC connection
             └─ 循环: scanf → execCMD → print prompt
                Loop: scanf → execCMD → print prompt
```

## 内存初始化时机 / Memory Init Timing

| 阶段 / Stage | 内存 / Memory | 说明 / Description |
|---|---|---|
| startup | DTCM, RAM_D1 | 栈 + FreeRTOS 静态内存 / Stack + FreeRTOS static mem |
| MX_FMC_Init | SDRAM 32MB (0xC0000000) | 硬件初始化 / Hardware init |
| MPU_Config | SDRAM Cacheable | 开启 SDRAM 缓存 / Enable SDRAM cache |
| MemControl_Init | TLSF 池 / TLSF pools | SDRAM 上 14MB 内核 + 10MB 用户 / 14MB kernel + 10MB user |

## 初始化顺序注意事项 / Init Order Notes

### MDMA 必须在 SDMMC 之前初始化 / MDMA Must Be Before SDMMC

FatFs 的 `f_mkfs` 函数需要 MDMA 支持。在 `hal_init.c` 的 `HAL_BoardInit()` 中,
`MX_MDMA_Init()` 必须在 `MX_SDMMC1_SD_Init()` 之前调用。

FatFs `f_mkfs` requires MDMA. In `HAL_BoardInit()` in `hal_init.c`,
`MX_MDMA_Init()` must be called before `MX_SDMMC1_SD_Init()`.

### FatFs 必须在 osKernelStart() 之后操作 / FatFs Must Be After osKernelStart()

FatFs 使用了 FreeRTOS 消息队列 (无 RTOS 模式下不需要)。
因此 `f_mount`, `f_mkfs` 等操作必须在调度器启动之后进行。

FatFs uses FreeRTOS message queues. Therefore `f_mount`, `f_mkfs` etc.
must be performed after the scheduler has started.

### devices_init() 只调用一次 / devices_init() Called Only Once

`devices_init()` 在 `DrTInit()` 内部通过 `Platform_DevicesInit()` 调用。
不要在 `freertos.c` 中重复调用, 否则设备树中每个设备会出现两次。

`devices_init()` is called inside `DrTInit()` via `Platform_DevicesInit()`.
Do NOT call it again in `freertos.c`, or each device will appear twice in the tree.

## 任务优先级 / Task Priorities

| 任务 / Task | 优先级 / Priority | 栈 / Stack | 说明 / Description |
|---|---|---|---|
| xNone (idle) | Normal | 512B | 空闲 LED 闪烁 / idle LED blink |
| xTaskInit (QueueInit) | Normal | 4KB | SD 挂载, 完成后挂起 / SD mount, then suspend |
| xShell | Normal | 4KB | USB CDC 终端 / USB CDC shell |
| xTaskManager | AboveNormal | 1KB | CPU 负载统计 (每秒) / CPU load stats per sec |
| xTest | Normal | 2KB | 测试 / test |
