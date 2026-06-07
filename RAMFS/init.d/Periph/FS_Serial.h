#ifndef H7OS_FS_SERIAL_H
#define H7OS_FS_SERIAL_H

#include "Core/DrT.h"

// Magic for flash header ("H7FS" in little-endian)
#define FS_SERIAL_MAGIC     0x53463748

// Record type tags
#define FS_RECORD_FS        0x01    // FS directory node
#define FS_RECORD_DEVICE    0x02    // DrTNode (device/file)
#define FS_RECORD_TASK      0x03    // Task (process)

// Sentinel index for NULL pointers
#define FS_INDEX_NULL       0xFFFF

/**
 * @brief Serialize the RAMFS tree to QSPI flash as binary records.
 *        Walks the tree, converts pointers to indices, writes with CRC32.
 */
void FS_Serialize(void);

/**
 * @brief Deserialize RAMFS tree from QSPI flash.
 *        Reads binary records, reconstructs FS nodes / devices / tasks.
 *        Must be called AFTER DrTInit() has created the base skeleton.
 */
void FS_Deserialize(void);

#endif // H7OS_FS_SERIAL_H
