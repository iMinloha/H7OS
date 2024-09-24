#include "echo_main.h"
#include "stdio.h"

void echo_main(int argc, char **argv){
    if (argc < 1){
        printf("Usage: echo <string>\n");
        return;
    }else{
        for (int i = 0; i < argc; i++){
            printf(argv[i]);
            if (i < argc - 1){
                printf(" ");
            }
        }
        printf("\n");
    }
}