#include <string.h>
#include "info_main.h"
#include "stdio.h"
#include "RAMFS.h"
#include "usbd_cdc_if.h"

extern CPU_t CortexM7;

void info_main(int argc, char **argv){
    if(argc == 0){
        USB_printf("info: missing argument\n");
    }else if (argc == 1){
        DrTNode_t drt = loadDevice(argv[0]);

        if(strcmp(drt->name, CortexM7->name) == 0) {
            showCPUInfo();
            return;
        }

        if(drt == NULL){
            Task_t task = loadTask(argv[0]);
            if(task == NULL){
                USB_printf("info: device not found\n");
                return;
            }else{
                USB_printf("Task\t\t\tPID\t\t\tPriority\t\t\tLoad\t\t\tStatus\n");
                USB_printf("%s\t\t%d", task->name, task->PID);
                switch (task->priority) {
                    case TASK_PRIORITY_NORMAL:
                        USB_printf("\t\t\tNormal");
                        break;
                    case TASK_PRIORITY_HIGH:
                        USB_printf("\t\t\tHigh");
                        break;
                    case TASK_PRIORITY_ROOT:
                        USB_printf("\t\t\tRoot");
                        break;
                    case TASK_PRIORITY_SYSTEM:
                        USB_printf("\t\t\tSystem");
                        break;
                    default:
                        USB_printf("\t\t\tUnknown");
                        break;
                }
                USB_printf("\t\t\t\t%f%%", task->cpu);
                switch (task->status) {
                    case TASK_READY:
                        USB_printf("\t\t\tReady\n");
                        break;
                    case TASK_RUNNING:
                        USB_printf("\t\t\tRunning\n");
                        break;
                    case TASK_SUSPEND:
                        USB_printf("\t\t\tSuspend\n");
                        break;
                    case TASK_STOP:
                        USB_printf("\t\t\tStop\n");
                        break;
                    default:
                        USB_printf("\t\t\tUnknown\n");
                        break;
                }
            }
            return;
        }else{
            USB_printf("Device: %s\n", drt->name);
            USB_printf("Description: %s\n", drt->description);
            switch (drt->type) {
                case DEVICE_TIMER:
                    USB_printf("Type: Timer\n");
                    break;
                case DEVICE_BS:
                    USB_printf("Type: Basic Device\n");
                    break;
                case DEVICE_STORAGE:
                    USB_printf("Type: Storage Device\n");
                    break;
                case DEVICE_DISPLAY:
                    USB_printf("Type: Display Device\n");
                    break;
                case DEVICE_INPUT:
                    USB_printf("Type: Input Device\n");
                    break;
                case DEVICE_SERIAL:
                    USB_printf("Type: Serial Device\n");
                    break;
                case DEVICE_TRANSPORT:
                    USB_printf("Type: Transport Device\n");
                    break;
                case DEVICE_VOTAGE:
                    USB_printf("Type: Voltage Device\n");
                    break;
                case FILE_SYSTEM:
                    USB_printf("Type: File System\n");
                    break;
                case DrTFILE:
                    USB_printf("Type: File\n");
                    break;
                default:
                    USB_printf("Type: Unknown\n");
                    break;
            }
            switch (drt->status) {
                case DEVICE_OFF:
                    USB_printf("Status: Off\n");
                    break;
                case DEVICE_ON:
                    USB_printf("Status: On\n");
                    break;
                case DEVICE_SUSPEND:
                    USB_printf("Status: Suspend\n");
                    break;
                case DEVICE_ERROR:
                    USB_printf("Status: Error\n");
                    break;
                case DEVICE_BUSY:
                    USB_printf("Status: Busy\n");
                    break;
                default:
                    USB_printf("Status: Unknown\n");
                    break;
            }
        }
    }else{
        USB_printf("info /path/device\n");
    }
    USB_printf("=-=-=-=-=-=-=-=-=-=-\n");
}