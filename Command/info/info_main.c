#include "info_main.h"
#include "stdio.h"
#include "RAMFS.h"

void info_main(int argc, char **argv){
    if(argc == 0){
        printf("info: missing argument\n");
    }else if (argc == 1){
        DrTNode_t drt = loadDevice(argv[0]);
        if(drt == NULL){
            Task_t task = loadTask(argv[0]);
            if(task == NULL){
                printf("info: device not found\n");
                return;
            }else{
                printf("Task\t\t\tPID\t\t\tPriority\t\t\tLoad\t\t\tStatus\n");
                printf("%s\t\t%d", task->name, task->PID);
                switch (task->priority) {
                    case TASK_PRIORITY_NORMAL:
                        printf("\t\t\tNormal");
                        break;
                    case TASK_PRIORITY_HIGH:
                        printf("\t\t\tHigh");
                        break;
                    case TASK_PRIORITY_ROOT:
                        printf("\t\t\tRoot");
                        break;
                    case TASK_PRIORITY_SYSTEM:
                        printf("\t\t\tSystem");
                        break;
                    default:
                        printf("\t\t\tUnknown");
                        break;
                }
                printf("\t\t\t\t%f%%", task->cpu);
                switch (task->status) {
                    case TASK_READY:
                        printf("\t\t\tReady\n");
                        break;
                    case TASK_RUNNING:
                        printf("\t\t\tRunning\n");
                        break;
                    case TASK_SUSPEND:
                        printf("\t\t\tSuspend\n");
                        break;
                    case TASK_STOP:
                        printf("\t\t\tStop\n");
                        break;
                    default:
                        printf("\t\t\tUnknown\n");
                        break;
                }
            }
            return;
        }else{
            printf("Device: %s\n", drt->name);
            printf("Description: %s\n", drt->description);
            switch (drt->type) {
                case DEVICE_TIMER:
                    printf("Type: Timer\n");
                    break;
                case DEVICE_BS:
                    printf("Type: Basic Device\n");
                    break;
                case DEVICE_STORAGE:
                    printf("Type: Storage Device\n");
                    break;
                case DEVICE_DISPLAY:
                    printf("Type: Display Device\n");
                    break;
                case DEVICE_INPUT:
                    printf("Type: Input Device\n");
                    break;
                case DEVICE_SERIAL:
                    printf("Type: Serial Device\n");
                    break;
                case DEVICE_TRANSPORT:
                    printf("Type: Transport Device\n");
                    break;
                case DEVICE_VOTAGE:
                    printf("Type: Voltage Device\n");
                    break;
                case FILE_SYSTEM:
                    printf("Type: File System\n");
                    break;
                case DrTFILE:
                    printf("Type: File\n");
                    break;
                default:
                    printf("Type: Unknown\n");
                    break;
            }
            switch (drt->status) {
                case DEVICE_OFF:
                    printf("Status: Off\n");
                    break;
                case DEVICE_ON:
                    printf("Status: On\n");
                    break;
                case DEVICE_SUSPEND:
                    printf("Status: Suspend\n");
                    break;
                case DEVICE_ERROR:
                    printf("Status: Error\n");
                    break;
                case DEVICE_BUSY:
                    printf("Status: Busy\n");
                    break;
                default:
                    printf("Status: Unknown\n");
                    break;
            }
        }
    }else{
        printf("info /path/device\n");
    }
    printf("=-=-=-=-=-=-=-=-=-=-\n");
}