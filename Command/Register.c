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
#include "save/save_main.h"
#include "pwd/pwd_main.h"
#include "rm/rm_main.h"
#include "reset/reset_main.h"
#include "flash/flash_main.h"
#include "touch/touch_main.h"
#include "mount/mount_main.h"
#include "cat/cat_main.h"
#include "use/use_main.h"

void register_main(){
    CMD("ls",    "List files",             "ls [path]",            ls_main);
    CMD("cd",    "Change directory",       "cd <path>",            cd_main);
    CMD("info",  "Show device/task info",  "info <path>",          info_main);
    CMD("echo",  "Print a message",        "echo <text>",          echo_main);
    CMD("help",  "Show command help",      "help [command]",       help_main);
    CMD("tree",  "Show filesystem tree",   "tree [depth]",         tree_main);
    CMD("mkdir", "Create a directory",     "mkdir <path>",         mkdir_main);
    CMD("mount", "Mount SD to RAMFS",      "mount [sd] <ramfs>",   mount_main);
    CMD("reboot","Restart the system",     "reboot",               reboot_main);
    CMD("save",  "Save filesystem to flash","save",                save_main);
    CMD("pwd",   "Print working directory","pwd",                  pwd_main);
    CMD("rm",    "Remove file/directory",  "rm <path>",            rm_main);
    CMD("reset", "Factory reset (erase)",  "reset",                reset_main);
    CMD("flash", "Erase QSPI flash chip",  "flash",                flash_main);
    CMD("touch", "Create a file",          "touch <path> [data]",  touch_main);
    CMD("cat",   "Print file contents",    "cat <file>",           cat_main);
    CMD("use",   "Use device fops",        "use <dev> <op> [args]", use_main);
}
