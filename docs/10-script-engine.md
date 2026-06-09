# 脚本引擎 / Script Engine (Builder)

Builder 是一个轻量嵌入式脚本解释器。每个脚本在**独立 FreeRTOS 线程**中异步运行，不阻塞 Shell，支持 `kill` 终止。

源码位于 `RAMFS/init.d/Bin/builder/`。

## 语法 / Syntax

```
# 注释

# ── 变量赋值 ──────────────────────────────────
var = 1
val = 0

# ── 设备 I/O (直接调用软件层 dev_* 函数) ─────
open /dev/gpio/PH7          → dev_open()
close /dev/gpio/PH7         → dev_close()
read /dev/serial/USART1 64  → dev_read()
write /dev/serial/USART1 hello  → dev_write()

# ── 计数循环 (含两端) ─────────────────────────
for <var> <start>:<end>
  ...
endfor

# ── 条件分支 ─────────────────────────────────
if <var> is <value>
  ...
else          # 可选
  ...
endif

# ── 延时 (毫秒, 让出 CPU) ───────────────────
delay(100)

# ── 其他 Shell 命令 (execCMD 分发) ──────────
echo hello
cat /readme
```

## 命令 / Commands

| 命令 | 用法 | 说明 |
|------|------|------|
| `run <file>` | `run /blink`, `run /sd/test.txt` | 异步启动脚本 |
| `kill <name>` | `kill /blink` | 终止运行中的脚本 |
| `kill <pid>` | `kill 3` | 按 PID 终止任务 |

## 架构 / Architecture

```
run /blink
  │
  ├─ 读取脚本文件 (RAMFS 或 SD)
  │
  └─ script_run_async(script, name)
       │
       ├─ 创建 Task 节点 → addThread() → /proc 可见
       ├─ 创建 FreeRTOS 线程 (osPriorityBelowNormal)
       └─ script_thread()
            │
            ├─ exec_range()  递归下降解释器
            │    ├─ L_OPEN   → dev_open()       (软件层, 不经过 execCMD)
            │    ├─ L_CLOSE  → dev_close()
            │    ├─ L_READ   → dev_read()
            │    ├─ L_WRITE  → dev_write()
            │    ├─ L_DELAY  → osDelay()         (精确延时)
            │    ├─ L_ASSIGN → var_set()
            │    ├─ L_FOR    → 循环体 (每轮检查 kill 标志)
            │    ├─ L_IF     → 条件分支
            │    └─ L_CMD    → subst() → execCMD() (Shell 命令)
            │
            └─ 清理: 从 /proc 移除, 释放内存

kill /blink
  │
  └─ script_kill(name)
       ├─ killed = 1  (优雅退出标志)
       └─ osThreadTerminate()  (强制终止)
```

## 线程安全 / Thread Safety

- 脚本线程优先级 `osPriorityBelowNormal`，低于 Shell (`osPriorityNormal`) — Shell 不会被阻塞
- `kill` 设置 `killed` 标志 + `osThreadTerminate` 双保险
- `delay(ms)` 调用 `osDelay()` 让出 CPU，不影响调度器
- 延时精度 ~1ms (FreeRTOS tick 周期)

## 限制 / Limits

| 项目 | 限制 |
|------|------|
| 变量数量 | 32 |
| 变量名长度 | 15 字符 |
| 脚本行数 | 256 |
| 脚本文件大小 | 8 KB (SD), 128 字节 (RAMFS) |
| 同时运行 | 4 个脚本 |
| 线程栈 | 1 KB |
| 变量类型 | int (32-bit) |

## 测试脚本 / Test Script

```
# GPIO 闪烁 — PH7 输出方波, USART1 输出计数, 每 100ms 翻转
open /dev/gpio/PH7
open /dev/serial/USART1

val = 0
for i 1:20
  write /dev/serial/USART1 i
  write /dev/gpio/PH7 val
  if val is 0
     val = 1
  else
     val = 0
  delay(100)
endfor

write /dev/serial/USART1 done
close /dev/gpio/PH7
close /dev/serial/USART1
```

预期: USART1 输出 1~20, PH7 每 100ms 翻转, 最后输出 "done"。
