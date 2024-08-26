#include "ls_main.h"
#include "RAMFS.h"
#include "u_stdio.h"
#include "memctl.h"

extern FS_t currentFS;

void ls_main(int argc, char **argv){
    if (argc > 1)
        u_print("ls: too many arguments\r\n");
    else if (argc == 1)
        ram_ls(argv[0]);
    else{
        char *pwd = (char *)kernal_alloc(1024);
        ram_pwd(currentFS, pwd);
        ram_ls(pwd);
    }
}