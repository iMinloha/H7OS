#include <stddef.h>
#include <stdio.h>
#include "cd_main.h"
#include "RAMFS.h"
#include "usbd_cdc_if.h"

extern FS_t currentFS;

void cd_main(int argc, char **argv){
    if(argc == 1){
        FS_t temp = ram_cd(argv[0]);
        if (temp == NULL){
            USB_printf("cd: no such directory ");
            for (int i = 0; argv[0][i] != '\0'; i++) USB_printf("%c", argv[0][i]);
            USB_printf("\r\n");
        }
        else currentFS = temp;
    }else if (argc > 1){
        USB_printf("cd: too many arguments\r\n");
    }
}