#include "ls_main.h"
#include "RAMFS.h"
#include "u_stdio.h"

void ls_main(int argc, char **argv){
    if (argc > 1)
        u_print("ls: too many arguments\r\n");
    else if (argc == 1)
        ram_ls(argv[0]);
    else
        ram_ls("/");
}