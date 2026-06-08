#include "pwd_main.h"
#include "Core/DrT.h"
#include "usbd_cdc_if.h"

extern FS_t currentFS;

void pwd_main(int argc, char **argv)
{
    if (argc > 0) {
        USB_printf("pwd: too many arguments\n");
    } else {
        char path[256];
        ram_pwd(currentFS, path);
        USB_printf("%s\n", path);
    }
}
