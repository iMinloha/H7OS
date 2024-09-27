#include "help_main.h"
#include "RAMFS.h"
#include "stdio.h"
#include "usbd_cdc_if.h"

void help_main(int argc, char **argv){
    if(argc == 0){
        helpCMD("");
    }else if(argc == 1) {
        helpCMD(argv[0]);
    }else{
        USB_printf("Usage: help [command]\n");
    }
}