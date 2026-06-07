#include "Core/DrT.h"
#include "memctl.h"
#include <string.h>

extern FS_t currentFS;
/* ============================[ SD Mount Path Resolution ]=========================== */

char* fs_to_sd_path(const char *rel_path) {
    if (currentFS == NULL || currentFS->sd_mount_path == NULL) return NULL;

    char *base = currentFS->sd_cd_path ? currentFS->sd_cd_path : currentFS->sd_mount_path;
    uint32_t len = strlen(base) + 1;
    if (rel_path != NULL && rel_path[0] != '\0')
        len += 1 + strlen(rel_path);

    char *sd_path = kernel_alloc(len + 1);
    strcpy(sd_path, base);
    if (rel_path != NULL && rel_path[0] != '\0') {
        strcat(sd_path, "/");
        strcat(sd_path, rel_path);
    }
    return sd_path;
}
