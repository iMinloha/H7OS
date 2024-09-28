#ifndef H7OS_DFS_H
#define H7OS_DFS_H

// 特殊符号标记
#define SPACE " "
#define TAB "  "
#define T_SIGNAL "|-"
#define END_SIGNAL "`-"

// 深度优先搜索算法
/***
 * @brief 递归遍历文件系统
 * @param node 文件系统节点
 * @param level 当前节点深度
 * @param depth 需要遍历深度
 * */
void dfs(FS_t node, int level, int depth);

/***
 * @brief 深度优先搜索算法
 * @param node 文件系统节点
 * @param depth 需要遍历深度
 * */
void DFS(FS_t node, int depth);

#endif