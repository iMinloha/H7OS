# BSP 驱动模型 / BSP Driver Model

## file_ops 接口 / file_ops Interface

所有 BSP 驱动统一使用 Linux 风格的 `file_ops` 函数表:

```c
typedef struct bsp_file_ops {
    int  (*open)  (void *dev);                          // 打开设备
    int  (*close) (void *dev);                          // 关闭设备
    int  (*read)  (void *dev, uint8_t *buf, uint32_t len); // 读取
    int  (*write) (void *dev, const uint8_t *buf, uint32_t len); // 写入
} bsp_file_ops_t;
```

## 设备注册流程 / Device Registration Flow

```
1. 驱动实现 file_ops 函数 / driver implements file_ops
   └─ gpio_read(), gpio_write(), ...
2. device_init() 调用 `dev_register()` (Platform 层 `dev_register.h`) 注册
   `dev_register()` 内部封装 addDevice() + loadDevice() + fops 绑定
```

## 已有驱动 / Existing Drivers

### GPIO

- **文件**: `HAL/<board>/BSP/gpio/bsp_gpio.{h,c}`
- **设备数据结构**:
  ```c
  typedef struct {
      GPIO_TypeDef *port;
      uint16_t      pin;
      uint8_t       is_output;  // 1=output, 0=input
  } gpio_dev_t;
  ```
- **注册宏**: `GPIO_OUTPUT(port, pin, name)`  `GPIO_INPUT(port, pin, name)`
- **路径**: `/dev/gpio/<name>`  (如 `/dev/gpio/PH7`)
- **读**: 返回 '0' 或 '1'
- **写**: '0' = LOW, '1' = HIGH

### USART

- **文件**: `HAL/<board>/BSP/usart/bsp_usart.{h,c}`
- **路径**: `/dev/serial/USART1`
- **读**: 非阻塞, 从 256 字节环形缓冲区读取, 中断驱动接收
- **写**: 阻塞, `HAL_UART_Transmit`
- **打开**: 使能中断 + 启动 `HAL_UART_Receive_IT`

### PWM

- **文件**: `HAL/<board>/BSP/pwm/bsp_pwm.{h,c}`
- **设备数据结构**: `{ TIM_HandleTypeDef *htim; uint32_t channel; }`
- **注册宏**: `PWM_REGISTER(htim, channel, freq_hz, name)`
- **路径**: `/dev/pwm/<name>` (如 `/dev/pwm/PWM2_CH1`)
- **写**: 0-100 的数字字符串, 表示占空比百分比

### ADC

- **文件**: `HAL/<board>/BSP/adc/bsp_adc.{h,c}`
- **注册宏**: `ADC_REGISTER(hadc, name)`
- **路径**: `/dev/adc/<name>` (如 `/dev/adc/ADC1`)
- **读**: 校准→启动→轮询转换→返回原始值字符串
- **写**: 不支持

### I2C

- **文件**: `HAL/<board>/BSP/i2c/bsp_i2c.{h,c}`
- **注册宏**: `I2C_REGISTER(hi2c, name)`
- **路径**: `/dev/i2c/<name>` (如 `/dev/i2c/I2C1`)
- **读写**: 首字节为 7 位从机地址 (左移 1 位), 后面为数据

### CPU 监控 / CPU Monitor

- **文件**: `HAL/<board>/BSP/cpu/bsp_cpu.{h,c}`
- **路径**: `/dev/cpu/CPU`
- **读**: 返回格式化的 CPU 信息 (名称, 频率, 温度, 负载)
- **内部**: 使用 ADC3 + 内部温度传感器 + DWT 周期计数器

## 添加新驱动 / Adding a New Driver

1. 在 `HAL/<board>/BSP/<driver>/` 创建 `bsp_<name>.h` 和 `bsp_<name>.c`
2. 定义设备数据结构 (如 `xxx_dev_t`)
3. 实现 `bsp_file_ops_t` 函数
4. 实现 `xxx_device_init()` 调用 `addDevice()` 注册
5. 在 `bsp_devices.c` 的 `devices_init()` 中调用
6. 在 `board_<name>.h` 中添加 `BOARD_HAS_XXX` 开关
