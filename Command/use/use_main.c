#include "use_main.h"
#include "usbd_cdc_if.h"
#include "Core/DrT.h"
#include "memctl.h"
#include <string.h>
#include <stdlib.h>

static const char *dev_err(int r) {
    switch (r) {
    case -1: return "no such device";
    case -2: return "device busy (opened by another task)";
    case -3: return "device not opened";
    case -4: return "permission denied (not the owner)";
    default: return NULL;
    }
}

void use_main(int argc, char **argv) {
    if (argc < 2) {
        USB_printf("Usage: use <device> <op> [args]\n");
        USB_printf("  use /dev/USART1 open\n");
        USB_printf("  use /dev/USART1 close\n");
        USB_printf("  use /dev/USART1 write <text>\n");
        USB_printf("  use /dev/USART1 read <bytes>\n");
        return;
    }

    char *dev_path = argv[0];
    char *op = argv[1];

    if (strcmp(op, "open") == 0) {
        int r = dev_open(dev_path);
        if (r == 0) USB_printf("use: open %s → OK\n", dev_path);
        else        USB_color_printf(LIGHT_RED, "use: open %s → %s (%d)\n", dev_path, dev_err(r) ? dev_err(r) : "error", r);
    }
    else if (strcmp(op, "close") == 0) {
        int r = dev_close(dev_path);
        if (r == 0) USB_printf("use: close %s → OK\n", dev_path);
        else        USB_color_printf(LIGHT_RED, "use: close %s → %s (%d)\n", dev_path, dev_err(r) ? dev_err(r) : "error", r);
    }
    else if (strcmp(op, "write") == 0 && argc >= 3) {
        int r = dev_write(dev_path, (uint8_t*)argv[2], strlen(argv[2]));
        if (r >= 0) USB_printf("use: write %s %d bytes → OK\n", dev_path, (int)strlen(argv[2]));
        else        USB_color_printf(LIGHT_RED, "use: write %s → %s (%d)\n", dev_path, dev_err(r) ? dev_err(r) : "error", r);
    }
    else if (strcmp(op, "read") == 0 && argc >= 3) {
        int len = atoi(argv[2]);
        if (len <= 0 || len > 512) len = 64;
        uint8_t *buf = (uint8_t*)kernel_alloc(len + 1);
        memset(buf, 0, len + 1);
        int r = dev_read(dev_path, buf, len);
        if (r > 0) {
            USB_printf("use: read %s %d bytes:\n", dev_path, r);
            for (int i = 0; i < r; i++) USB_printf("%c", buf[i]);
            USB_printf("\n");
        } else {
            USB_color_printf(LIGHT_RED, "use: read %s → %s (%d)\n", dev_path, dev_err(r) ? dev_err(r) : "error", r);
        }
        kernel_free(buf);
    }
    else {
        USB_printf("use: unknown op '%s'\n", op);
    }
}
