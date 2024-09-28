#include <stdio.h>
#include "tree_main.h"
#include "RAMFS.h"

extern FS_t currentFS;


void tree_main(int argc, char **argv){
    if (argc == 0)
        DFS(currentFS, 1);
    else if (argc == 1){
        // 获取argv对应的FS_t
        uint8_t depth = (uint8_t) (argv[0] - '0');
        if (depth >= 0 && depth <= 9) DFS(currentFS, depth);
        else printf("Invalid depth\n");
    }
}