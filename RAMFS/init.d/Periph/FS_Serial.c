/**
 * FS_Serial.c — 完整 RAMFS 文件系统二进制持久化到 QSPI Flash
 *
 * 序列化所有 FS 目录节点 + DrTFILE 文件节点(含数据) 到 QSPI Flash。
 * 开机时从 Flash 读取并重建。
 *
 * Flash 布局:
 *   [4B: magic  "H7FS"]
 *   [4B: CRC32]
 *   [2B: dir_count] [2B: file_count]
 *   目录条目: [2B: path_len] [path incl. \0]
 *   文件条目: [2B: path_len] [path incl. \0] [2B: data_len] [data bytes]
 *   [4B: end_marker 0x454E4400]
 */

#include "FS_Serial.h"
#include "memctl.h"
#include "quadspi.h"
#include "usbd_cdc_if.h"

/* ================================================================== */
/* CRC32 (Ethernet 多项式, 查表法)                                      */
/* ================================================================== */

static uint32_t crc32_table[256];
static int crc32_ready = 0;

static void crc32_init(void) {
    if (crc32_ready) return;
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t crc = i;
        for (int j = 0; j < 8; j++) {
            crc = (crc >> 1) ^ (crc & 1 ? 0xEDB88320UL : 0);
        }
        crc32_table[i] = crc;
    }
    crc32_ready = 1;
}

static uint32_t crc32_compute(const uint8_t* data, uint32_t len) {
    crc32_init();
    uint32_t crc = 0xFFFFFFFFUL;
    for (uint32_t i = 0; i < len; i++) {
        crc = crc32_table[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
    }
    return crc ^ 0xFFFFFFFFUL;
}

/* ================================================================== */
/* 二进制读写辅助函数                                                    */
/* ================================================================== */

static void write_u16(uint8_t* buf, uint32_t* offset, uint16_t val) {
    buf[(*offset)++] = (uint8_t)(val & 0xFF);
    buf[(*offset)++] = (uint8_t)((val >> 8) & 0xFF);
}

static void write_u32(uint8_t* buf, uint32_t* offset, uint32_t val) {
    buf[(*offset)++] = (uint8_t)(val & 0xFF);
    buf[(*offset)++] = (uint8_t)((val >> 8) & 0xFF);
    buf[(*offset)++] = (uint8_t)((val >> 16) & 0xFF);
    buf[(*offset)++] = (uint8_t)((val >> 24) & 0xFF);
}

static uint16_t read_u16(const uint8_t* buf, uint32_t* offset) {
    uint16_t val = (uint16_t)buf[*offset] | ((uint16_t)buf[*offset + 1] << 8);
    *offset += 2;
    return val;
}

static uint32_t read_u32(const uint8_t* buf, uint32_t* offset) {
    uint32_t val = (uint32_t)buf[*offset]
        | ((uint32_t)buf[*offset + 1] << 8)
        | ((uint32_t)buf[*offset + 2] << 16)
        | ((uint32_t)buf[*offset + 3] << 24);
    *offset += 4;
    return val;
}

/* ================================================================== */
/* 文件条目结构 (用于收集 DrTFILE 节点)                                   */
/* ================================================================== */

typedef struct {
    char* path; /* 完整路径, kernel_alloc */
    uint8_t* data; /* 文件数据, kernel_alloc */
    uint16_t data_len;
} FileEntry;

/* ================================================================== */
/* 树遍历: 收集目录 + 文件                                               */
/* ================================================================== */

static void collect_tree(FS_t node, char* prefix, uint32_t plen, uint32_t pfx_size,
                         char** dirs, uint16_t* dir_count, uint16_t max_dirs,
                         FileEntry* files, uint16_t* file_count, uint16_t max_files) {
    if (node == NULL) return;

    /* --- 收集当前节点的 DrTFILE 文件 --- */
    DrTNode_t dev = node->node;
    while (dev != NULL && *file_count < max_files) {
        if (dev->type == DrTFILE) {
            /* 构建完整路径: prefix + "/" + dev->name */
            uint32_t path_len = plen + 1 + strlen(dev->name);
            char* full_path = (char*)kernel_alloc(path_len + 1);
            memoryCopy(full_path, prefix, plen);
            if (plen > 0 && prefix[plen - 1] != '/') {
                full_path[plen] = '/';
                plen++;
            }
            strcpy(full_path + plen, dev->name);
            plen = path_len; /* restore plen for calculation, but we're done */

            files[*file_count].path = full_path;
            /* 复制文件数据 (最多 127 字节有效载荷) */
            uint16_t dlen = 0;
            if (dev->data != NULL) {
                /* 找实际数据长度 (最大 127) */
                uint8_t* d = (uint8_t*)dev->data;
                while (dlen < 127 && d[dlen] != '\0') dlen++;
                if (dlen > 0) {
                    files[*file_count].data = (uint8_t*)kernel_alloc(dlen);
                    memoryCopy(files[*file_count].data, dev->data, dlen);
                }
                else {
                    files[*file_count].data = NULL;
                }
            }
            else {
                files[*file_count].data = NULL;
            }
            files[*file_count].data_len = dlen;
            (*file_count)++;
        }
        dev = dev->next;
    }

    /* --- 递归遍历子目录 --- */
    FS_t child = node->child_next;
    while (child != NULL && *dir_count < max_dirs) {
        uint32_t need = plen + 1 + strlen(child->path) + 1;
        if (need > pfx_size) {
            child = child->level_next;
            continue;
        }

        char saved_prefix[256];
        memoryCopy(saved_prefix, prefix, plen + 1);

        if (plen > 0 && prefix[plen - 1] != '/') {
            prefix[plen] = '/';
            plen++;
        }
        strcpy(prefix + plen, child->path);
        uint32_t new_plen = plen + strlen(child->path);

        /* 记录目录路径 */
        char* path_copy = (char*)kernel_alloc(new_plen + 1);
        memoryCopy(path_copy, prefix, new_plen);
        path_copy[new_plen] = '\0';
        dirs[*dir_count] = path_copy;
        (*dir_count)++;

        /* 递归进入子节点 */
        collect_tree(child, prefix, new_plen, pfx_size,
                     dirs, dir_count, max_dirs,
                     files, file_count, max_files);

        /* 恢复前缀 */
        memoryCopy(prefix, saved_prefix, plen + 1);
        child = child->level_next;
    }
}

/* ================================================================== */
/* FS_Serialize — 遍历整棵树，写入 QSPI Flash                           */
/* ================================================================== */

#define FS_SERIAL_MAX_ITEMS      256
#define FS_SERIAL_BUF_SIZE       8192

void FS_Serialize(void) {
    char** dirs = (char**)kernel_alloc(sizeof(char*) * FS_SERIAL_MAX_ITEMS);
    FileEntry* files = (FileEntry*)kernel_alloc(sizeof(FileEntry) * FS_SERIAL_MAX_ITEMS);
    /* Mount entries: [0]=ramfs_path, [1]=sd_path */
    char** mnt_ramfs = (char**)kernel_alloc(sizeof(char*) * FS_SERIAL_MAX_ITEMS);
    char** mnt_sd    = (char**)kernel_alloc(sizeof(char*) * FS_SERIAL_MAX_ITEMS);
    char* prefix = (char*)kernel_alloc(512);
    memset(prefix, 0, 512);
    uint16_t dir_count = 0, file_count = 0, mount_count = 0;

    /* 收集目录和文件 */
    collect_tree(RAM_FS, prefix, 0, 512,
                 dirs, &dir_count, FS_SERIAL_MAX_ITEMS,
                 files, &file_count, FS_SERIAL_MAX_ITEMS);

    /* 收集挂载点 */
    {
        char mnt_prefix[256];
        strcpy(mnt_prefix, "");
        FS_t stack[64]; int sp = 0;
        stack[sp++] = RAM_FS;
        while (sp > 0 && mount_count < FS_SERIAL_MAX_ITEMS) {
            FS_t node = stack[--sp];
            if (node->sd_mount_path) {
                /* Build absolute RAMFS path */
                char abs_path[256] = "";
                FS_t cur = node;
                int depth = 0; FS_t chain[32];
                while (cur && cur != RAM_FS) { chain[depth++] = cur; cur = cur->parent; }
                for (int d = depth - 1; d >= 0; d--) {
                    strcat(abs_path, "/");
                    strcat(abs_path, chain[d]->path);
                }
                if (abs_path[0] == '\0') strcpy(abs_path, "/");
                mnt_ramfs[mount_count] = (char*)kernel_alloc(strlen(abs_path) + 1);
                strcpy(mnt_ramfs[mount_count], abs_path);
                mnt_sd[mount_count] = (char*)kernel_alloc(strlen(node->sd_mount_path) + 1);
                strcpy(mnt_sd[mount_count], node->sd_mount_path);
                mount_count++;
            }
            FS_t child = node->child_next;
            FS_t rev[64]; int rc = 0;
            while (child) { rev[rc++] = child; child = child->level_next; }
            for (int r = rc - 1; r >= 0 && sp < 64; r--) stack[sp++] = rev[r];
        }
    }

    /* 构建二进制 blob */
    uint8_t* blob = (uint8_t*)kernel_alloc(FS_SERIAL_BUF_SIZE);
    memset(blob, 0, FS_SERIAL_BUF_SIZE);
    uint32_t offset = 0;

    write_u32(blob, &offset, FS_SERIAL_MAGIC);

    uint32_t crc_offset = offset;
    write_u32(blob, &offset, 0x00000000); /* CRC32 占位 */

    write_u16(blob, &offset, dir_count);
    write_u16(blob, &offset, file_count);
    write_u16(blob, &offset, mount_count);

    /* 目录条目 */
    for (uint16_t i = 0; i < dir_count; i++) {
        uint16_t plen = (uint16_t)strlen(dirs[i]) + 1;
        write_u16(blob, &offset, plen);
        memoryCopy(blob + offset, dirs[i], plen);
        offset += plen;
        if (offset > FS_SERIAL_BUF_SIZE - 1024) break;
    }

    /* 文件条目 */
    for (uint16_t i = 0; i < file_count; i++) {
        uint16_t plen = (uint16_t)strlen(files[i].path) + 1;
        write_u16(blob, &offset, plen);
        memoryCopy(blob + offset, files[i].path, plen);
        offset += plen;
        write_u16(blob, &offset, files[i].data_len);
        if (files[i].data_len > 0) {
            memoryCopy(blob + offset, files[i].data, files[i].data_len);
            offset += files[i].data_len;
        }
        if (offset > FS_SERIAL_BUF_SIZE - 1024) break;
    }

    /* 挂载点条目 */
    for (uint16_t i = 0; i < mount_count; i++) {
        uint16_t plen = (uint16_t)strlen(mnt_ramfs[i]) + 1;
        write_u16(blob, &offset, plen);
        memoryCopy(blob + offset, mnt_ramfs[i], plen);
        offset += plen;
        uint8_t sd_len = (uint8_t)strlen(mnt_sd[i]);
        blob[offset++] = sd_len;
        if (sd_len > 0) {
            memoryCopy(blob + offset, mnt_sd[i], sd_len);
            offset += sd_len;
        }
        if (offset > FS_SERIAL_BUF_SIZE - 1024) break;
    }

    /* 计算 CRC32 (不包含结束标记) */
    uint32_t payload_len = offset - (crc_offset + 4);
    uint32_t crc = crc32_compute(blob + crc_offset + 4, payload_len);
    {
        uint32_t patch_off = crc_offset;
        write_u32(blob, &patch_off, crc);
    }

    /* 结束标记 */
    write_u32(blob, &offset, 0x454E4400);

    USB_printf("save: %d dirs, %d files, %d mounts → ", dir_count, file_count, mount_count);
    if (QSPI_W25Qxx_ChipErase() != QSPI_W25Qxx_OK) {
        USB_color_printf(LIGHT_RED, "erase failed!\n");
        goto cleanup;
    }
    USB_printf("erase OK, writing... ");
    if (QSPI_W25Qxx_WriteBuffer(blob, 0, offset) != QSPI_W25Qxx_OK) {
        USB_color_printf(LIGHT_RED, "write failed!\n");
        goto cleanup;
    }
    USB_printf("verify... ");
    uint8_t* vb = (uint8_t*)kernel_alloc(FS_SERIAL_BUF_SIZE);
    memset(vb, 0, FS_SERIAL_BUF_SIZE);
    if (QSPI_W25Qxx_ReadBuffer(vb, 0, offset) == QSPI_W25Qxx_OK && memoryCompare(blob, vb, offset)) {
        USB_color_printf(LIGHT_GREEN, "OK (%lu bytes)\n", offset);
    }
    else {
        USB_color_printf(LIGHT_RED, "mismatch!\n");
    }
    kernel_free(vb);

cleanup:
    for (uint16_t i = 0; i < dir_count; i++) kernel_free(dirs[i]);
    for (uint16_t i = 0; i < file_count; i++) {
        kernel_free(files[i].path);
        if (files[i].data) kernel_free(files[i].data);
    }
    for (uint16_t i = 0; i < mount_count; i++) {
        kernel_free(mnt_ramfs[i]);
        kernel_free(mnt_sd[i]);
    }
    kernel_free(dirs);
    kernel_free(files);
    kernel_free(mnt_ramfs);
    kernel_free(mnt_sd);
    kernel_free(prefix);
    kernel_free(blob);
}

/* ================================================================== */
/* FS_Deserialize — 从 Flash 读取，重建目录和文件                         */
/* ================================================================== */

void FS_Deserialize(void) {
    uint8_t header[16];
    if (QSPI_W25Qxx_ReadBuffer(header, 0, 16) != QSPI_W25Qxx_OK) return;

    uint32_t hdr_off = 0;
    uint32_t magic = read_u32(header, &hdr_off);
    if (magic != FS_SERIAL_MAGIC) return;

    uint32_t stored_crc = read_u32(header, &hdr_off);
    uint16_t dir_count   = read_u16(header, &hdr_off);
    uint16_t file_count  = read_u16(header, &hdr_off);
    uint16_t mount_count = read_u16(header, &hdr_off);

    if (dir_count == 0 && file_count == 0 && mount_count == 0) return;

    uint8_t* blob = (uint8_t*)kernel_alloc(FS_SERIAL_BUF_SIZE);
    memset(blob, 0, FS_SERIAL_BUF_SIZE);
    if (QSPI_W25Qxx_ReadBuffer(blob, 0, FS_SERIAL_BUF_SIZE) != QSPI_W25Qxx_OK) {
        kernel_free(blob);
        return;
    }

    /* 计算 payload 长度: 遍历所有条目 */
    uint32_t entry_off = 14; /* 跳过 magic(4)+crc(4)+dir(2)+file(2)+mount(2) */
    for (uint16_t i = 0; i < dir_count; i++) {
        uint16_t plen = read_u16(blob, &entry_off);
        entry_off += plen;
        if (entry_off >= FS_SERIAL_BUF_SIZE) goto done;
    }
    for (uint16_t i = 0; i < file_count; i++) {
        uint16_t plen = read_u16(blob, &entry_off);
        entry_off += plen;
        uint16_t dlen = read_u16(blob, &entry_off);
        entry_off += dlen;
        if (entry_off >= FS_SERIAL_BUF_SIZE) goto done;
    }
    for (uint16_t i = 0; i < mount_count; i++) {
        uint16_t plen = read_u16(blob, &entry_off);
        entry_off += plen;
        if (entry_off >= FS_SERIAL_BUF_SIZE) goto done;
        uint8_t sd_len = blob[entry_off++];
        entry_off += sd_len;
        if (entry_off >= FS_SERIAL_BUF_SIZE) goto done;
    }

    /* 验证 CRC32 */
    {
        uint32_t payload_len = entry_off - 8; /* payload_start = 8 */
        uint32_t computed_crc = crc32_compute(blob + 8, payload_len);
        if (computed_crc != stored_crc) {
            USB_color_printf(LIGHT_RED, "[FS]: CRC mismatch\n");
            goto done;
        }
    }

    /* 重建目录 */
    uint32_t off = 14;
    for (uint16_t i = 0; i < dir_count; i++) {
        uint16_t plen = read_u16(blob, &off);
        char* check_copy = (char*)kernel_alloc(plen + 1);
        memoryCopy(check_copy, blob + off, plen);
        check_copy[plen] = '\0';

        char* dir_path = (char*)kernel_alloc(plen + 1);
        strcpy(dir_path, check_copy);
        off += plen;

        FS_t existing = loadPath(check_copy);
        kernel_free(check_copy);
        if (existing == NULL) {
            ram_deep_mkdir(dir_path);
        }
        kernel_free(dir_path);
    }

    /* 重建文件 */
    for (uint16_t i = 0; i < file_count; i++) {
        uint16_t plen = read_u16(blob, &off);
        char* file_path = (char*)kernel_alloc(plen + 1);
        memoryCopy(file_path, blob + off, plen);
        file_path[plen] = '\0';
        off += plen;

        uint16_t dlen = read_u16(blob, &off);
        uint8_t* fdata = (dlen > 0) ? (blob + off) : NULL;
        off += dlen;

        ram_touch(file_path, fdata, dlen);
        kernel_free(file_path);
    }

    /* 重建挂载点 */
    for (uint16_t i = 0; i < mount_count; i++) {
        uint16_t plen = read_u16(blob, &off);
        char* ramfs_path = (char*)kernel_alloc(plen + 1);
        memoryCopy(ramfs_path, blob + off, plen);
        ramfs_path[plen] = '\0';
        off += plen;

        uint8_t sd_len = blob[off++];
        char* sd_path = NULL;
        if (sd_len > 0) {
            sd_path = (char*)kernel_alloc(sd_len + 1);
            memoryCopy(sd_path, blob + off, sd_len);
            sd_path[sd_len] = '\0';
            off += sd_len;
        }

        /* Find the RAMFS directory and set mount path */
        char* check_copy = (char*)kernel_alloc(strlen(ramfs_path) + 1);
        strcpy(check_copy, ramfs_path);
        FS_t mnt_node = loadPath(check_copy);
        kernel_free(check_copy);

        if (mnt_node) {
            if (mnt_node->sd_mount_path) kernel_free(mnt_node->sd_mount_path);
            mnt_node->sd_mount_path = sd_path ? sd_path : kernel_alloc(1);
            if (!sd_path) mnt_node->sd_mount_path[0] = '\0';
        } else {
            if (sd_path) kernel_free(sd_path);
        }
        kernel_free(ramfs_path);
    }

done:
    kernel_free(blob);
}
