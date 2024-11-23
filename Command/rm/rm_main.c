#include "rm_main.h"
#include "usbd_cdc_if.h"
#include "init.d/DrT/DrT.h"
#include "memctl.h"
#include "init.d/CPU/CS.h"

extern FS_t currentFS;

void rm(char *path, Bool isDir){
    // dir拼接为绝对路径
    char *dir = kernel_alloc(128);
    ram_pwd(currentFS, dir);

    // 拼接dir和path
    if (path[0] == '/' || strcmp(dir, "/") == 0) sprintf(dir, "%s%s", dir, path);
    else sprintf(dir, "%s/%s", dir, path);


    // 删除目标
    ram_rm(dir, isDir);

    // 拼接指令保存在CS树中
    char *cmd = kernel_alloc(96);

    if (isDir == True) sprintf(cmd, "rm -r %s", dir);
    else sprintf(cmd, "rm %s", dir);

    CS_push(cmd);

    // 释放内存
    kernel_free(dir);
    kernel_free(cmd);
}



/***                                                                                    
 * @note: rm path/filename
 *        rm -r path
 * */
void rm_main(int argc, char *argv[]){
    Bool isDir = strcmp(argv[0], "-r") == 0 ? True : False;
    if (argc == 0) USB_printf("Usage: rm <path>/<file> or rm -r <path>\n");
    else if (argc == 1){
        // 从后往前分割路径和文件名
        if (isDir == True) USB_printf("Usage: rm <path>/<file> or rm -r <path>\n");
        else rm(argv[0], isDir);
    }else if (argc == 2){
        if (isDir == True) rm(argv[1], isDir);
        else USB_printf("Usage: rm <path>/<file> or rm -r <path>\n");
    } else USB_printf("Usage: rm <path>/<file> or rm -r <path>\n");
}