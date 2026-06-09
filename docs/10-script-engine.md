# Builder 脚本引擎 / Builder Script Engine

> **H7OS** 嵌入式脚本解释器，异步执行、支持循环/条件/算术、直接操作外设。

源码: `RAMFS/init.d/Bin/builder/script.c` `script.h`

---

## 目录 / Table of Contents

- [快速开始 / Quick Start](#快速开始--quick-start)
- [命令参考 / Commands](#命令参考--commands)
  - [run — 启动脚本](#run--启动脚本)
  - [kill — 终止脚本](#kill--终止脚本)
- [语法参考 / Syntax Reference](#语法参考--syntax-reference)
  - [变量与赋值](#变量与赋值)
  - [算术运算符](#算术运算符)
  - [设备 I/O](#设备-io)
  - [十六进制字节](#十六进制字节)
  - [条件分支 if](#条件分支-if)
  - [循环 for / while](#循环-for--while)
  - [延时 delay](#延时-delay)
  - [Shell 命令](#shell-命令)
  - [注释](#注释)
- [外设用法 / Peripherals](#外设用法--peripherals)
  - [GPIO](#gpio)
  - [USART 串口](#usart-串口)
  - [PWM](#pwm)
  - [ADC](#adc)
  - [I2C](#i2c)
  - [CPU](#cpu)
  - [完整示例: ADC → PWM](#完整示例-adc--pwm)
- [架构设计 / Architecture](#架构设计--architecture)
  - [分层结构](#分层结构)
  - [对象模型: 行信息 (li_t)](#对象模型-行信息-li_t)
  - [对象模型: 行解析器 (li_parse)](#对象模型-行解析器-li_parse)
  - [对象模型: 递归下降解释器 (exec_range)](#对象模型-递归下降解释器-exec_range)
  - [对象模型: 脚本任务 (script_job_t)](#对象模型-脚本任务-script_job_t)
  - [数据流: 一行脚本的完整生命周期](#数据流-一行脚本的完整生命周期)
- [扩展指南 / Extension Guide](#扩展指南--extension-guide)
  - [添加新的 I/O 关键字](#添加新的-io-关键字)
  - [添加新的控制流关键字](#添加新的控制流关键字)
  - [添加新的变量操作符](#添加新的变量操作符)
- [线程模型 / Threading](#线程模型--threading)
- [限制 / Limits](#限制--limits)

---

## 快速开始 / Quick Start

```bash
# 创建脚本文件
touch /myscript "
val = 0
for i 1:10
  write /dev/gpio/PF7 val
  if val == 0
     val = 1
  else
     val = 0
  endif
  delay(500)
endfor
"

# 运行
run /myscript

# 查看状态
info /proc/myscript
tree /proc

# 终止
kill /myscript
```

---

## 命令参考 / Commands

### run — 启动脚本

| 用法 | 说明 |
|------|------|
| `run /path/to/script` | 从 RAMFS 或 SD 卡异步执行脚本 |

脚本在独立 FreeRTOS 线程中运行，不阻塞 Shell。

### kill — 终止脚本

| 用法 | 说明 |
|------|------|
| `kill <name>` | 按名称终止脚本任务 |
| `kill /proc/<name>` | 按 /proc 路径终止 |
| `kill <pid>` | 按 PID 终止 |

---

## 语法参考 / Syntax Reference

### 变量与赋值

```
var = 42              # 整数赋值
var = 1 + 2           # 加法 → 3
var = a + b           # 变量加法
var = count * 2       # 乘法
var = total / 10      # 整数除法
count++               # 自增 (count = count + 1)
count--               # 自减 (count = count - 1)
adc = read /dev/adc/ADC1 16   # 读外设值存入变量
```

变量作用域为整个脚本，最多 32 个，类型为 `int` (32-bit)。

### 算术运算符

| 运算符 | 示例 | 说明 |
|:---:|------|------|
| `+` | `a + b` | 加法 |
| `-` | `a - b` | 减法 |
| `*` | `a * b` | 乘法 |
| `/` | `a / b` | 除法 (整数, 除零返回 0) |
| `++` | `a++` | 自增 |
| `--` | `a--` | 自减 |

> 表达式运算前会先做变量替换。`a + b` 中 `a`, `b` 会被替换为当前值后再计算。

### 设备 I/O

```
open /dev/gpio/PF7                 # 打开设备 (可省略, write/read 自动 open)
close /dev/gpio/PF7                # 关闭设备
write /dev/gpio/PF7 1              # 写入
write /dev/serial/USART1 hello     # 写入文本
read /dev/adc/ADC1 16              # 读取
val = read /dev/gpio/PF7 1         # 读取并存入变量 val

use /dev/gpio/PF7 write 1          # 兼容旧语法
```

### 十六进制字节

```
write /dev/i2c/I2C1 0x50 0x01 0x02   # 发送 3 字节: [0x50, 0x01, 0x02]
write /dev/serial/USART1 hello 0x0D 0x0A  # 文本 + CRLF
```

`0xNN` 序列被解析为单字节，普通字符保持 ASCII。

### 条件分支 if

```
if var == 0
   ...
else if var > 10       # 嵌套 if
   ...
else
   ...
endif
```

| 运算符 | 别名 |
|:---:|:---:|
| `==` | `is` |
| `!=` | — |
| `>` | — |
| `<` | — |
| `>=` | — |
| `<=` | — |

### 循环 for / while

```
# 计数循环 (含两端)
for i 1:10
  write /dev/serial/USART1 i
  delay(100)
endfor

# 条件循环 (变量比较)
while val == 0
  val = read /dev/gpio/PF7 1
  delay(50)
endwhile

# 无限循环
while 1
  write /dev/gpio/PF7 1
  delay(500)
  write /dev/gpio/PF7 0
  delay(500)
endwhile
```

循环可任意嵌套，每轮迭代检查 `kill` 标志。

### 延时 delay

```
delay(100)    # 延时 100 ms (调用 osDelay, 让出 CPU)
```

### Shell 命令

```
echo hello world        # 输出文本
cat /readme.txt         # 查看文件
tree /sd                # 显示目录树
```

任何 Shell 命令均可直接使用，通过 `execCMD()` 分发。

### 注释

```
# 这是注释
var = 1   # 行尾注释 (不支持, 请单独一行)
```

---

## 外设用法 / Peripherals

所有外设统一 `open` → `write`/`read` → `close`，脚本引擎自动 `open`。

### GPIO

| 路径 | write | read |
|------|-------|------|
| `/dev/gpio/PF7` | `0` / `1` / `0x01` | 电平 `0` 或 `1` |
| `/dev/gpio/PH7` | `0` / `1` (LED) | 同上 |

```python
write /dev/gpio/PF7 1     # 高电平
write /dev/gpio/PF7 0     # 低电平
val = read /dev/gpio/PF7 1   # 读取电平
```

### USART 串口

| 路径 | write | read |
|------|-------|------|
| `/dev/serial/USART1` | 文本字符串 | 字节数 |

```python
write /dev/serial/USART1 Hello from H7OS
data = read /dev/serial/USART1 64   # 最多读 64 字节
```

### PWM

| 路径 | write (占空比 0-100) | 频率 |
|------|:---:|:---:|
| `/dev/pwm/PWM2_CH1` | `0`-`100` | 50Hz (固件固定) |
| `/dev/pwm/PWM2_CH2` | `0`-`100` | 50Hz |

```python
write /dev/pwm/PWM2_CH1 75    # 75% 占空比
write /dev/pwm/PWM2_CH1 0     # 停止输出
```

### ADC

| 路径 | read |
|------|------|
| `/dev/adc/ADC1` | 电压值 (字符串 → int) |

```python
val = read /dev/adc/ADC1 16    # 读取 ADC 存入变量
if val > 2000
   write /dev/pwm/PWM2_CH1 80
endif
```

### I2C

| 路径 | write |
|------|-------|
| `/dev/i2c/I2C1` | 二进制 (首字节=设备地址) |

```python
write /dev/i2c/I2C1 0x50 0x01 0x02   # 写设备 0x50, 数据 0x01 0x02
```

### CPU

| 路径 | read |
|------|------|
| `/dev/cpu/CPU` | CPU 状态信息字符串 |

### 完整示例: ADC → PWM

```python
# 根据 ADC 读数调节 PWM 占空比
while 1
  val = read /dev/adc/ADC1 16
  if val > 2000
     write /dev/pwm/PWM2_CH1 80
  else if val > 1000
     write /dev/pwm/PWM2_CH1 50
  else
     write /dev/pwm/PWM2_CH1 10
  endif
  delay(100)
endwhile
```

---

## 架构设计 / Architecture

### 分层结构

```
┌──────────────────────────────────────┐
│  script_run_async()   公共 API       │  ← 入口
├──────────────────────────────────────┤
│  li_parse()           词法分析器      │  ← 行→ li_t 对象
│  parse_bin()          十六进制解析    │
│  parse_cmp()          比较运算符解析   │
│  eval_expr()          算术表达式求值   │
├──────────────────────────────────────┤
│  exec_range()         递归下降解释器   │  ← 块执行引擎
│  exec_line()          单行执行器       │
│  exec_for/if/while    控制流执行器    │
├──────────────────────────────────────┤
│  subst()              变量替换器       │  ← 数据流
│  va/vf()              变量表 CRUD     │
├──────────────────────────────────────┤
│  dev_open/read/write  软件层 I/O      │  ← 底层
│  execCMD()            Shell 命令分发   │
└──────────────────────────────────────┘
```

### 对象模型: 行信息 (li_t)

每一行脚本被解析为一个 `li_t` (Line Info) 对象：

```c
typedef enum { L_CMD, L_ASSIGN, L_FOR, L_IF, L_WHILE, L_DELAY,
               L_OPEN, L_CLOSE, L_READ, L_WRITE, L_READ_VAR, L_INC } lk_t;

typedef struct {
    lk_t kind;          // ← 多态类型标记
    char dev[128];      // 设备路径 (I/O 操作)
    char buf[256];      // 替换后的命令行 / 表达式
    int  v_int;         // 整型参数 (delay_ms / read_len / cmp_val / inc_val)
    cmp_t if_op;        // 比较运算符 (IF / WHILE)
    char fv[16];        // FOR 循环变量名
    int  fs, fe;        // FOR 起止值
    char iv[16];        // IF/WHILE 变量名 / L_READ_VAR 目标变量 / L_INC 变量
} li_t;
```

**设计模式**：Variants (Tagged Union)。`kind` 字段决定哪些成员有效——类似 C++ 的 `std::variant` 或 Rust 的 `enum`。

| kind | 有效字段 |
|------|---------|
| `L_OPEN/CLOSE` | `dev` |
| `L_READ` | `dev`, `v_int` |
| `L_READ_VAR` | `dev`, `v_int`, `iv` |
| `L_WRITE` | `dev` |
| `L_DELAY` | `v_int` |
| `L_ASSIGN` | `iv` (变量名), `buf` (表达式) |
| `L_INC` | `iv` (变量名), `v_int` (±1) |
| `L_FOR` | `fv`, `fs`, `fe` |
| `L_IF/WHILE` | `iv`, `v_int`, `if_op` |
| `L_CMD` | `buf` |

### 对象模型: 行解析器 (li_parse)

```c
void li_parse(li_t *info, const char *raw)
```

**职责**：工厂函数，输入一行原始文本，输出一个 `li_t` 对象。

**算法**：优先级匹配链 (Chain of Responsibility)
```
raw文本
  ├─ sscanf("delay(%d)")          → L_DELAY
  ├─ sscanf("open %s")            → L_OPEN
  ├─ sscanf("close %s")           → L_CLOSE
  ├─ sscanf("read %s %d")         → L_READ
  ├─ sscanf("write %s")           → L_WRITE
  ├─ sscanf("%s = read %s %d")    → L_READ_VAR
  ├─ sscanf("use %s %s")          → 兼容旧语法
  ├─ sscanf("for %s %d:%d")       → L_FOR
  ├─ sscanf("if %s %s %d") + cmp  → L_IF
  ├─ sscanf("while %s %s %d")+cmp → L_WHILE
  ├─ sscanf("while %d")           → L_WHILE (无限)
  ├─ sscanf("%s++")               → L_INC (+1)
  ├─ sscanf("%s--")               → L_INC (-1)
  ├─ sscanf("%s %c") + '='        → L_ASSIGN (表达式)
  └─ fallback                     → L_CMD
```

匹配顺序很重要——更具体的模式必须在更通用的模式之前。例如 `var = read ...` 必须在 `var = expr` 之前匹配。

### 对象模型: 递归下降解释器 (exec_range)

```
exec_range(L, start, end)         ← 执行行范围
  │
  ├─ for 行 → exec_for()         ← 创建循环迭代器
  │   └─ exec_range()              (递归执行循环体)
  │
  ├─ while 行 → exec_while()
  │   └─ exec_range()
  │
  ├─ if 行 → exec_if()
  │   ├─ cond true  → exec_range() (if 块)
  │   └─ cond false → exec_range() (else 块)
  │
  └─ 普通行 → exec_line()
      ├─ L_OPEN   → dev_open()
      ├─ L_WRITE  → subst() → parse_bin() → dev_write()
      ├─ L_ASSIGN → eval_expr() → var_set()
      └─ L_CMD    → execCMD()
```

**设计模式**：Interpreter + Composite。`exec_range` 递归调用自身处理嵌套块——这就是为什么 `for`/`while`/`if` 可以任意嵌套。

### 对象模型: 脚本任务 (script_job_t)

```c
typedef struct {
    osThreadId  handle;       // FreeRTOS 线程句柄
    Task_t      task;         // RAMFS 任务节点 (/proc 可见)
    char        name[20];     // 脚本名称
    script_ctx_t ctx;         // 执行上下文 (kill 标志, 错误计数)
} script_job_t;
```

全局 `_jobs[MAX_JOBS]` 数组管理所有运行中的脚本。每个 job 对应一个 FreeRTOS 线程 + 一个 `/proc` 任务节点。

**生命周期**:
```
script_run_async()
  ├─ new job[i] → addThread() → 注册到 /proc
  ├─ osThreadCreate() → 启动线程
  │
  └─ script_thread()
       ├─ exec_range() 执行脚本
       ├─ task->status = TASK_STOP      (正常退出)
       └─ job[i] removed from _jobs[]

script_kill()
  ├─ ctx->killed = 1
  ├─ osThreadTerminate()               (强制终止)
  ├─ 从 /proc 移除任务
  └─ job[i] removed
```

### 数据流: 一行脚本的完整生命周期

```
"write /dev/gpio/PF7 0x01"
     │
     ▼
li_parse()  ──────────────────────────────────────────
     │  sscanf("write %127s", path) → path="/dev/gpio/PF7"
     │  subst(dev, path)             → dev="/dev/gpio/PF7"
     │  kind = L_WRITE
     ▼
li_t { .kind=L_WRITE, .dev="/dev/gpio/PF7" }
     │
     ▼
exec_line()
     │  dev_open("/dev/gpio/PF7") → 自动 open
     │  write_data_ptr(raw)       → data="0x01"
     │  subst(ds, data)           → ds="0x01" (无变量替换)
     │  parse_bin(bin, ds)        → bin=[0x01]
     │  dev_write(dev, bin, 1)    → HAL_GPIO_WritePin(..., SET)
     ▼
GPIO 引脚输出高电平 ✓
```

---

## 扩展指南 / Extension Guide

Builder 采用**数据驱动 + 模式匹配**设计，添加新语法只需改动 3 个地方。

### 添加新的 I/O 关键字

以添加 `pwm <dev> <duty>` 为例：

**Step 1** — 在 `lk_t` 枚举添加新类型：
```c
typedef enum { ..., L_PWM } lk_t;
```

**Step 2** — 在 `li_parse()` 添加匹配规则（在 fallback `L_CMD` 之前）：
```c
/* pwm <dev> <duty> */
char path[128]; int duty;
if (sscanf(raw, "pwm %127s %d", path, &duty) == 2) {
    info->kind = L_PWM;
    subst(info->dev, sizeof(info->dev), path);   // 替换设备路径中的变量
    info->v_int = duty;                           // 占空比
    return;
}
```

**Step 3** — 在 `exec_line()` 的 switch 中添加执行逻辑：
```c
case L_PWM: {
    r = dev_open(in->dev);
    if (r == 0 || r == -2) {
        char duty_str[16];
        int n = snprintf(duty_str, sizeof(duty_str), "%d", in->v_int);
        r = dev_write(in->dev, (const uint8_t*)duty_str, n);
    }
    break;
}
```

### 添加新的控制流关键字

以添加 `repeat <count> ... endrepeat` 为例：

**Step 1** — 枚举：
```c
typedef enum { ..., L_REPEAT } lk_t;
```

**Step 2** — `li_parse()`：
```c
if (sscanf(raw, "repeat %d", &v1) == 1) {
    info->kind = L_REPEAT;
    info->v_int = v1;
    return;
}
```

**Step 3** — 添加 `find_endrepeat()` 块边界查找 (类似 `find_endfor`)。

**Step 4** — 在 `exec_range()` 添加：
```c
} else if (!strncmp(ln, "repeat ", 7)) {
    exec_repeat(ctx, L, i);
    i = find_endrepeat(L, i);
    ...
} else if (!strcmp(ln, "endrepeat")) {
    i++;
```

**Step 5** — 实现 `exec_repeat()`：
```c
static void exec_repeat(script_ctx_t *ctx, lines_t *L, int i) {
    li_t info; li_parse(&info, L->lines[i]);
    int tail = find_endrepeat(L, i);
    for (int k = 0; k < info.v_int && !ctx->killed; k++) {
        exec_range(ctx, L, i + 1, tail);
    }
}
```

### 添加新的变量操作符

以添加 `var += n` (复合赋值) 为例：

**Step 1** — `li_parse()` 中匹配：
```c
char op2[4]; int val;
if (sscanf(raw, "%15s %2s %d", vn, op2, &val) == 3 && !strcmp(op2, "+=")) {
    info->kind = L_INC;
    strncpy(info->iv, vn, VN_MAX-1);
    info->v_int = val;   // 增量值
    return;
}
```

`exec_line` 中已有的 `L_INC` 分支自动处理：`x->v += in->v_int`。

---

## 线程模型 / Threading

| 属性 | 值 |
|------|-----|
| 执行方式 | 独立 FreeRTOS 线程 |
| 优先级 | `osPriorityBelowNormal` (低于 Shell) |
| 栈大小 | 1024 字节 |
| 最大并发 | 4 个脚本 |
| 互斥保护 | `taskENTER_CRITICAL()` (TLSF) + `strtok_r` (命令解析) |
| 终止 | `kill` 设置标志 + `osThreadTerminate` |
| 调度 | Shell 忙等循环每 1ms `osDelay(1)` 让出 CPU |

---

## 限制 / Limits

| 项目 | 限制 |
|------|------|
| 变量数量 | 32 |
| 变量名长度 | 15 字符 |
| 脚本行数 | 256 |
| 脚本文件大小 | 8 KB (SD), 128 B (RAMFS) |
| 同时运行 | 4 个脚本 |
| 线程栈 | 1024 字节 (`SCRIPT_STACK_SZ`) |
| 变量类型 | `int` (32-bit, 有符号) |
| 表达式 | 单个二元运算 (`a op b`) |
| 嵌套深度 | 受栈制约 (~5 层) |
| 条件运算符 | `==`, `!=`, `>`, `<`, `>=`, `<=` |
| 算术运算符 | `+`, `-`, `*`, `/`, `++`, `--` |
