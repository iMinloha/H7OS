# 启动流程 / Initialization Flow

## 完整启动序列 / Full Boot Sequence

```
Reset / 复位
  │
  ▼
startup_stm32h743iitx.s        ← 向量表, 栈初始化, 跳转 main
  │
  ▼
main.c: main()
  ├─ 1. SCB_EnableICache()             ← 指令缓存
  ├─ 2. SCB_EnableDCache()             ← 数据缓存
  ├─ 3. HAL_Init()                     ← HAL 库, SysTick
  │
  ├─ 4. HAL_BoardInit()                ← HAL 层 (hal_init.c)
  │      ├─ MPU_Config()               ←   配置 SDRAM Cacheable
  │      ├─ SystemClock_Config()        ←   HSE 25MHz → PLL1 → 480MHz
  │      ├─ PeriphCommonClock_Config()  ←   PLL2 → ADC 80MHz
  │      ├─ MX_GPIO_Init()             ←   所有外设硬件初始化
  │      ├─ MX_DMA_Init()
  │      ├─ MX_MDMA_Init()
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
  ├─ 5. Platform_Init()                ← Platform 层
  │      └─ board_periph_init()        ←   板级外设
  │           ├─ DFU 检测 (RTC 备份寄存器)
  │           ├─ MX_USB_DEVICE_Init()  ←   USB CDC
  │           ├─ BSP_SD_Init()         ←   SD 卡硬件
  │           └─ Touch_Init()          ←   触摸屏
  │
  ├─ 6. MX_FREERTOS_Init()            ← FreeRTOS + OS 任务
  │      ├─ osThreadCreate(xNone)      ←   空闲任务
  │      ├─ MemControl_Init()          ←   TLSF 分配器 (SDRAM)
  │      ├─ taskGlobalInit()           ←   全局初始化
  │      │    ├─ DrTInit()             ←     DrT 设备树
  │      │    │    └─ devices_init()   ←     注册 BSP 设备到 /dev
  │      │    ├─ SD 卡检测 (board_sd_get_capacity)
  │      │    └─ QSPI Flash 初始化
  │      ├─ devices_init() (重复, 无害) / (duplicate, harmless)
  │      └─ ThreadInit()               ←   创建 OS 任务
  │           ├─ xTaskInit  → QueueInit
  │           ├─ xShell     → ShellTask
  │           ├─ xTaskManager → TaskManager
  │           └─ xTest → testFunc
  │
  └─ 7. osKernelStart()               ← FreeRTOS 调度器启动
        │
        ├─ QueueInit 任务 (一次性):
        │    ├─ FS_Deserialize()       ←   从 QSPI 还原 RAMFS
        │    ├─ f_mount(SD)            ←   挂载 SD 卡
        │    ├─ f_mkfs(SD) if needed   ←   如需要, 格式化 SD
        │    └─ 挂起 (osDelay 循环)
        │
        └─ ShellTask 任务:
             ├─ 等待 USB CDC 连接
             └─ 循环: scanf → execCMD → print prompt
```

## 内存初始化时机 / Memory Init Timing

| 阶段 | 内存 | 说明 |
|------|------|------|
| startup | DTCM, RAM_D1 | 栈 + FreeRTOS 静态内存 |
| MX_FMC_Init | SDRAM 32MB (0xC0000000) | 硬件初始化 |
| MPU_Config | SDRAM Cacheable | 开启 SDRAM 缓存 |
| MemControl_Init | TLSF 池 | SDRAM 上 14MB 内核 + 10MB 用户 |

## 任务优先级 / Task Priorities

| 任务 | 优先级 | 栈 | 说明 |
|------|--------|-----|------|
| xNone (idle) | Normal | 512B | 空闲 LED 闪烁 / idle LED blink |
| xTaskInit (QueueInit) | Normal | 4KB | SD 挂载, 完成后挂起 / SD mount, then suspend |
| xShell | Normal | 4KB | USB CDC 终端 / USB CDC shell |
| xTaskManager | AboveNormal | 1KB | CPU 负载统计 (每秒) / CPU load stats per sec |
| xTest | Normal | 2KB | 测试 / test |
