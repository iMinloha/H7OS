#include "ls_main.h"
#include "RAMFS.h"
#include "stdio.h"
#include "memctl.h"
#include "usbd_cdc_if.h"

extern FS_t currentFS;

void ls_main(int argc, char **argv){
    if (argc > 1)
        USB_printf("ls: too many arguments\r\n");
    else if (argc == 1)
        ram_ls(argv[0]);
    else{
        char *pwd = (char *)kernel_alloc(1024);
        ram_pwd(currentFS, pwd);
        ram_ls(pwd);
    }
}