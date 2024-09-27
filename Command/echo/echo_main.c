#include "echo_main.h"
#include "stdio.h"
#include "usbd_cdc_if.h"

void echo_main(int argc, char **argv){
    if (argc < 1){
        USB_printf("Usage: echo <string>\n");
        return;
    }else{
        for (int i = 0; i < argc; i++){
            USB_printf(argv[i]);
            if (i < argc - 1){
                USB_printf(" ");
            }
        }
        USB_printf("\n");
    }
}