#include "pwd_main.h"
#include "init.d/DrT/DrT.h"
#include "usbd_cdc_if.h"
#include "memctl.h"

extern FS_t currentFS;

void pwd_main(int argc, char **argv){
    if (argc > 0) USB_printf("pwd: too many arguments");
    else{
        char *path = kernel_alloc(128);
        ram_pwd(currentFS, path);
        USB_printf("%s\n", path);
    }
}