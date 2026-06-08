#include "ls_main.h"
#include "RAMFS.h"
#include "memctl.h"
#include "usbd_cdc_if.h"

extern FS_t currentFS;

void ls_main(int argc, char **argv)
{
    if (argc > 1) {
        USB_printf("ls: too many arguments\n");
    } else if (argc == 1) {
        ram_ls(argv[0]);
    } else {
        char pwd[256];
        ram_pwd(currentFS, pwd);
        ram_ls(pwd);
    }
}
