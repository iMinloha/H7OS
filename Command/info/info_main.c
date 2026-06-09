#include <string.h>
#include "info_main.h"
#include "RAMFS.h"
#include "usbd_cdc_if.h"
#include "memctl.h"
#include "bsp_file_ops.h"

extern CPU_t CortexM7;

static void print_task_info(Task_t task)
{
    USB_printf("%-16s %-5s %-12s %-8s %-10s\n",
        "Task", "PID", "Priority", "Load", "Status");
    USB_printf("-------------------------------------------------\n");
    USB_printf("%-16s %-5u", task->name, task->PID);
    switch (task->priority) {
    case TASK_PRIORITY_NORMAL:  USB_printf(" %-12s", "Normal"); break;
    case TASK_PRIORITY_HIGH:    USB_printf(" %-12s", "High");   break;
    case TASK_PRIORITY_ROOT:    USB_printf(" %-12s", "Root");   break;
    case TASK_PRIORITY_SYSTEM:  USB_printf(" %-12s", "System"); break;
    default:                   USB_printf(" %-12s", "Unknown");break;
    }
    USB_printf(" %-7.1f%%", task->cpu);
    switch (task->status) {
    case TASK_READY:    USB_printf(" %-10s\n", "Ready");   break;
    case TASK_RUNNING:  USB_printf(" %-10s\n", "Running"); break;
    case TASK_SUSPEND:  USB_printf(" %-10s\n", "Suspend"); break;
    case TASK_STOP:     USB_printf(" %-10s\n", "Stop");    break;
    default:           USB_printf(" %-10s\n", "Unknown"); break;
    }
}

static void print_dev_type(DeviceType_E type)
{
    switch (type) {
    case DEVICE_TIMER:      USB_printf("Type: Timer\n");           break;
    case DEVICE_BS:         USB_printf("Type: Basic Device\n");    break;
    case DEVICE_STORAGE:    USB_printf("Type: Storage Device\n");  break;
    case DEVICE_DISPLAY:    USB_printf("Type: Display Device\n");  break;
    case DEVICE_INPUT:      USB_printf("Type: Input Device\n");    break;
    case DEVICE_SERIAL:     USB_printf("Type: Serial Device\n");   break;
    case DEVICE_TRANSPORT:  USB_printf("Type: Transport Device\n");break;
    case DEVICE_VOTAGE:     USB_printf("Type: Voltage Device\n");  break;
    case FILE_SYSTEM:       USB_printf("Type: File System\n");     break;
    case DrTFILE:           USB_printf("Type: File\n");            break;
    case APP:              USB_printf("Type: Executable\n");      break;
    default:               USB_printf("Type: Unknown\n");         break;
    }
}

static void print_dev_status(DeviceStatus_E status)
{
    switch (status) {
    case DEVICE_OFF:     USB_printf("Status: Off\n");     break;
    case DEVICE_ON:      USB_printf("Status: On\n");      break;
    case DEVICE_SUSPEND: USB_printf("Status: Suspend\n"); break;
    case DEVICE_ERROR:   USB_printf("Status: Error\n");   break;
    case DEVICE_BUSY:    USB_printf("Status: Busy\n");    break;
    default:            USB_printf("Status: Unknown\n"); break;
    }
}

void info_main(int argc, char **argv)
{
    if (argc == 0) {
        USB_printf("Usage: info <device|task>\n");
        return;
    }
    if (argc > 1) {
        USB_printf("info: too many arguments\n");
        return;
    }

    DrTNode_t drt = loadDevice(argv[0]);
    if (drt) {
        USB_printf("Device: %s\n", drt->name);
        USB_printf("Description: %s\n", drt->description);
        print_dev_type(drt->type);
        print_dev_status(drt->status);

        /* 诊断读取: 直接调用 fops->read, 绕过所有权检查 */
        if (drt->fops) {
            bsp_file_ops_t *f = (bsp_file_ops_t*)drt->fops;
            uint8_t buf[256];
            int n = f->read ? f->read(drt->device, buf, sizeof(buf) - 1) : -1;
            if (n > 0) {
                buf[n] = '\0';
                USB_printf("Value: %s\n", (char *) buf);
            }
        }
        USB_printf("=-=-=-=-=-=-=-=-=-=-\n");
        return;
    }

    Task_t task = loadTask(argv[0]);
    if (task) {
        print_task_info(task);
        return;
    }

    USB_printf("info: '%s' not found\n", argv[0]);
}
