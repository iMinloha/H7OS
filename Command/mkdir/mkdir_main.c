#include <stdio.h>
#include "mkdir_main.h"
#include "init.d/DrT/DrT.h"
#include "usbd_cdc_if.h"
#include "memctl.h"
#include "usart.h"
#include "init.d/CPU/CS.h"

void mkdir_main(int argc, char **argv){
    // ¹¦ÄÜ
    if (argc == 1) {
        FS_t create = ram_deep_mkdir(argv[0]);
        char *buf = kernel_alloc(48);
        ram_pwd(create, buf);
        char *cmd = kernel_alloc(96);
        sprintf(cmd, "mkdir %s", buf);
        kernel_free(buf);
        CS_push(cmd);
        kernel_free(cmd);
    }
    else USB_printf("Usage: mkdir -path\n");
}