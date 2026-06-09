#include "kill_main.h"
#include "script.h"
#include "Core/DrT.h"
#include "usbd_cdc_if.h"
#include <string.h>
#include <stdlib.h>

void kill_main(int argc, char **argv) {
    if (argc < 1) {
        USB_printf("Usage: kill <name|pid|/proc/name>\n");
        script_list();
        return;
    }

    char *target = argv[0];

    /* ── 去掉 /proc/ 前缀 ─────────────────────────── */
    if (!strncmp(target, "/proc/", 6)) target += 6;
    else if (!strncmp(target, "proc/", 5)) target += 5;

    /* ── 1. 按名称杀脚本 ──────────────────────────── */
    if (script_kill(target) == 0) {
        USB_printf("killed '%s'\n", target);
        return;
    }

    /* ── 2. 按 PID 杀任务 ──────────────────────────── */
    int pid = atoi(target);
    if (pid > 0) {
        Task_t t = getThreadByPID((uint8_t)pid);
        if (t && t->handle) {
            osThreadTerminate(t->handle);
            USB_printf("killed PID %d\n", pid);
            return;
        }
    }

    USB_color_printf(LIGHT_RED, "kill: '%s' not found\n", target);
}
