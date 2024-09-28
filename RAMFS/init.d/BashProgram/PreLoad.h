#ifndef H7OS_PRELOAD_H
#define H7OS_PRELOAD_H

#include "RAMFS.h"

// 用户内存空间分配
#define user_malloc(size) ram_alloc(size)
#define user_free(ptr) ram_free(ptr)



#endif
