#include "Register.h"
#include "RAMFS.h"
#include "ls/ls_main.h"
#include "cd/cd_main.h"

void register_main(){
    // ×¢²áÖ¸Áî
    CMD("ls", "List files", ls_main);
    CMD("cd", "Change Directory", cd_main);
}