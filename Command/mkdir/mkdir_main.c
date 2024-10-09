#include <stdio.h>
#include "mkdir_main.h"
#include "init.d/DrT/DrT.h"
#include "usbd_cdc_if.h"

void mkdir_main(int argc, char **argv){
    // ¹¦ÄÜ
    if (argc == 1) ram_deep_mkdir(argv[0]);
    else USB_printf("Usage: mkdir -path\n");
}