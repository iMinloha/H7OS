#include "mount_main.h"
#include "usbd_cdc_if.h"
#include "Core/DrT.h"
#include "memctl.h"
#include "fatfs.h"
#include <string.h>

extern FS_t currentFS;
extern FATFS SDFatFS;

void mount_main(int argc, char *argv[]){
    if (argc < 1 || argc > 2) {
        USB_printf("Usage: mount <sd_path> <ramfs_path>\n");
        USB_printf("       mount <ramfs_path>   (SD root -> ramfs)\n");
        return;
    }

    if (SDFatFS.fs_type == 0) {
        USB_color_printf(LIGHT_RED, "mount: SD card not available\n");
        return;
    }

    /* SD 路径: "/" 或单参数模式默认 SD 根目录 */
    char *sd_path;
    char *ramfs_rel;
    if (argc == 1) {
        sd_path = "";
        ramfs_rel = argv[0];
    } else {
        sd_path = argv[0];
        ramfs_rel = argv[1];
    }
    if (sd_path[0] == '/' && sd_path[1] == '\0') sd_path = "";

    /* 验证 SD 路径存在 */
    DIR test_dir;
    FRESULT fr = f_opendir(&test_dir, sd_path);
    if (fr != FR_OK) {
        USB_color_printf(LIGHT_RED, "mount: SD path '%s' error=%d\n", sd_path[0] ? sd_path : "/", fr);
        return;
    }
    f_closedir(&test_dir);

    /* 拼接 RAMFS 绝对路径 */
    char *ramfs_abs = kernel_alloc(256);
    if (ramfs_rel[0] == '/') {
        strcpy(ramfs_abs, ramfs_rel);
    } else {
        ram_pwd(currentFS, ramfs_abs);
        uint32_t len = strlen(ramfs_abs);
        if (len > 0 && ramfs_abs[len - 1] != '/') {
            ramfs_abs[len] = '/';
            ramfs_abs[len + 1] = '\0';
        }
        strcat(ramfs_abs, ramfs_rel);
    }

    /* 创建 RAMFS 目录 */
    char *check = kernel_alloc(strlen(ramfs_abs) + 1);
    strcpy(check, ramfs_abs);
    FS_t existing = loadPath(check);
    kernel_free(check);

    FS_t mnt_fs;
    if (existing) {
        mnt_fs = existing;
    } else {
        mnt_fs = ram_deep_mkdir(ramfs_abs);
        if (!mnt_fs) {
            USB_color_printf(LIGHT_RED, "mount: failed to create %s\n", ramfs_abs);
            kernel_free(ramfs_abs);
            return;
        }
    }

    /* 设置挂载点 */
    if (mnt_fs->sd_mount_path) kernel_free(mnt_fs->sd_mount_path);
    mnt_fs->sd_mount_path = kernel_alloc(strlen(sd_path) + 1);
    strcpy(mnt_fs->sd_mount_path, sd_path);

    USB_printf("mount: %s -> %s\n", sd_path[0] ? sd_path : "/", ramfs_abs);
    kernel_free(ramfs_abs);
}
