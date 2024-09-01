#include "echo_main.h"
#include "u_stdio.h"

void echo_main(int argc, char **argv){
    if (argc < 1){
        u_print("Usage: echo <string>\n");
        return;
    }else{
        for (int i = 0; i < argc; i++){
            u_print(argv[i]);
            if (i < argc - 1){
                u_print(" ");
            }
        }
        u_print("\n");
    }
}