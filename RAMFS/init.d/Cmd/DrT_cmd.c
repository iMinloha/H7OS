#include "Core/DrT.h"
#include "memctl.h"
#include "usbd_cdc_if.h"
#include <string.h>
// ============================[ָ指令系统]===========================
void addCMD(char* name, char* description, char* usage, Comand_t cmd) {
    CMD_t p = CMDList;
    while (p->next != NULL) p = p->next;
    CMD_t newCMD = (CMD_t)kernel_alloc(sizeof(struct CMD));

    newCMD->name = (char*)kernel_alloc(strlen(name) + 1);
    newCMD->description = (char*)kernel_alloc(strlen(description) + 1);
    newCMD->usage = (char*)kernel_alloc(strlen(usage) + 1);
    strcpy(newCMD->name, name);
    strcpy(newCMD->description, description);
    strcpy(newCMD->usage, usage);

    newCMD->cmd = cmd;
    newCMD->next = NULL;
    p->next = newCMD;
}

// 执行指令
void execCMD(char* command_rel) {
    // 使用指令前需要先声明一个内存空间用于保护指令所处空间的安全
    char* command = (char*)kernel_alloc(strlen(command_rel) + 1);
    strcpy(command, command_rel);

    // 分割指令
    char* argv[128] = {0};
    int argc = 0;
    char* token = strtok(command, " ");
    while (token != NULL) {
        // 遍历并分割
        argv[argc++] = token;
        token = strtok(NULL, " ");
    }

    argc -= 1;


    CMD_t p = CMDList->next;
    while (p != NULL) {
        if (strcmp(p->name, argv[0]) == 0) {
            p->cmd(argc, &argv[1]);
            kernel_free(command);
            return;
        }
        p = p->next;
    }

    USB_printf("sh: '%s' not found. Type 'help'.\n", argv[0]);
    kernel_free(command);
}

void helpCMD(char* cmd) {
    char buf[128];
    memoryCopy(buf, cmd, strlen(cmd) + 1);
    CMD_t p = CMDList->next;
    if (buf[0] == '\0') {
        USB_printf("%-8s  %-26s  %s\n", "Command", "Description", "Usage");
        while (p != NULL) {
            USB_printf("%-8s  %-26s  %s\n", p->name, p->description, p->usage);
            p = p->next;
        }
    }
    else {
        while (p != NULL) {
            if (strcmp(p->name, cmd) == 0) {
                USB_printf("%s — %s\n", p->name, p->description);
                USB_printf("Usage: %s\n", p->usage);
                return;
            }
            p = p->next;
        }
    }
}
