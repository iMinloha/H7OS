#include "Register.h"
#include "RAMFS.h"
#include "ls/ls_main.h"
#include "cd/cd_main.h"
#include "info/info_main.h"

void register_main(){
    // ×¢²áÖ¸Áî¼¯
    CMD("ls", "List files", "ls /path or ls", ls_main);
    CMD("cd", "Change Directory", "cd /path", cd_main);
    CMD("info", "list something infomation", "info path/file", info_main);
}