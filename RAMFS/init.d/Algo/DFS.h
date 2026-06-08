#ifndef H7OS_DFS_H
#define H7OS_DFS_H

#include "Core/DrT.h"

/** Display filesystem tree with proper branch lines.
 *  @param node  starting FS node (usually currentFS)
 *  @param depth max recursion depth
 */
void DFS(FS_t node, int depth);

#endif
