#include "info_main.h"
#include "u_stdio.h"
#include "RAMFS.h"

void info_main(int argc, char **argv){
    if(argc == 0){
        u_print("info: missing argument\n");
    }else if (argc == 1){
        DrTNode_t drt = loadDevice(argv[0]);
        if(drt == NULL){
            u_print("info: device not found\n");
            return;
        }else{
            u_print("Device: %s\n", drt->name);
            u_print("Description: %s\n", drt->description);
            switch (drt->type) {
                case DEVICE_TIMER:
                    u_print("Type: Timer\n");
                    break;
                case DEVICE_BS:
                    u_print("Type: Basic Device\n");
                    break;
                case DEVICE_STORAGE:
                    u_print("Type: Storage Device\n");
                    break;
                case DEVICE_DISPLAY:
                    u_print("Type: Display Device\n");
                    break;
                case DEVICE_INPUT:
                    u_print("Type: Input Device\n");
                    break;
                case DEVICE_SERIAL:
                    u_print("Type: Serial Device\n");
                    break;
                case DEVICE_TRANSPORT:
                    u_print("Type: Transport Device\n");
                    break;
                case DEVICE_VOTAGE:
                    u_print("Type: Voltage Device\n");
                    break;
                case FILE_SYSTEM:
                    u_print("Type: File System\n");
                    break;
                case FILE:
                    u_print("Type: File\n");
                    break;
                default:
                    u_print("Type: Unknown\n");
                    break;
            }
            switch (drt->status) {
                case DEVICE_OFF:
                    u_print("Status: Off\n");
                    break;
                case DEVICE_ON:
                    u_print("Status: On\n");
                    break;
                case DEVICE_SUSPEND:
                    u_print("Status: Suspend\n");
                    break;
                case DEVICE_ERROR:
                    u_print("Status: Error\n");
                    break;
                case DEVICE_BUSY:
                    u_print("Status: Busy\n");
                    break;
                default:
                    u_print("Status: Unknown\n");
                    break;
            }
        }
    }else{
        u_print("info /path/device\n");
    }
    u_print("=-=-=-=-=-=-=-=-=-=-\n");
}