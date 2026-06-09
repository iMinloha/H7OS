#ifndef H7OS_SCRIPT_H
#define H7OS_SCRIPT_H

#include "cmsis_os.h"

/* ── 异步脚本执行 ──────────────────────────────────── */

/* 在独立线程中运行脚本, 返回线程句柄 (NULL=失败).
 * 脚本任务自动注册到 RAMFS 任务列表, info 可见. */
osThreadId script_run_async(const char *script, const char *name);

/* ── 脚本控制 ──────────────────────────────────────── */

/* 终止指定名称的脚本任务, 返回 0=成功 */
int script_kill(const char *name);

/* 列出所有运行中的脚本 (name pid) */
void script_list(void);

#endif
