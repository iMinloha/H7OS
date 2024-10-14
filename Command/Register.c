#include "Register.h"
#include "RAMFS.h"
#include "ls/ls_main.h"
#include "cd/cd_main.h"
#include "info/info_main.h"
#include "echo/echo_main.h"
#include "help/help_main.h"
#include "tree/tree_main.h"
#include "mkdir/mkdir_main.h"
#include "reboot/reboot_main.h"
#include "clear/clear_main.h"
#include "cmd/cmd_main.h"
#include "save/save_main.h"
#include "dfu/dfu_main.h"

void register_main(){
    // ×¢²áÖ¸Áî¼¯(bash, ls, cd, info, echo, help, tree)
    CMD("ls", "List files", "ls -path or ls", ls_main);
    CMD("cd", "Change Directory", "cd -path", cd_main);
    CMD("info", "list something infomation", "info -path/file", info_main);
    CMD("echo", "echo something", "echo your want print things", echo_main);
    CMD("help", "help using command", "help -command", help_main);
    CMD("tree", "show file system struct tree", "tree -depth", tree_main);
    CMD("mkdir", "make your owner folder", "mkdir -path", mkdir_main);
    CMD("reboot", "restart the microcontroller", "reboot", reboot_main);
    CMD("save", "safely save the operating system", "save", save_main);
//    CMD("dfu", "the system enters DFU mode", "dfu", dfu_main);
//    CMD("cmd", "nothing happend", "cmd", cmd_main);
//    CMD("clear", "clean uart screen", "clear", clear_main);
}