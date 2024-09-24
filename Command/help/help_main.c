#include "help_main.h"
#include "RAMFS.h"
#include "stdio.h"

void help_main(int argc, char **argv){
    if(argc == 0){
        helpCMD("");
    }else if(argc == 1) {
        helpCMD(argv[0]);
    }else{
        printf("Usage: help [command]\n");
    }
}