#include <stdio.h>
#include "tree_main.h"
#include "RAMFS.h"
#include "usbd_cdc_if.h"

extern FS_t currentFS;


void tree_main(int argc, char **argv){
    USB_printf("\n");
    if (argc == 0) DFS(currentFS, 1);
    else if (argc == 1) DFS(currentFS, atoi(argv[0]));
    else USB_printf("Invalid number of arguments, use command: tree -depth\n");
}