#include <stdio.h>
#include "RAMFS.h"

#define ShowTab(level) for (int time_tick_temp = 0; time_tick_temp < level; time_tick_temp++) printf(TAB);

/***
 * @brief 递归遍历文件系统
 * @param node 文件系统节点
 * @param level 当前节点深度
 * @param depth 需要遍历深度
 * */
void dfs(FS_t node, int level, int depth){
    // 递归遍历文件系统

    // 1. 打印当前节点下的设备
    DrTNode_t device = node->node;
    for (int i = 0;i < node->node_count; i++){
        ShowTab(level);
        if (i == node->node_count - 1) printf("%s%s\n", END_SIGNAL, device->name);
        else printf("%s%s\n", T_SIGNAL, device->name);
        device = device->next;
    }

    // 2. 打印当前节点下的任务列表
    Task_t task = node->tasklist;
    while (task != NULL){
        ShowTab(level);
        if (task->next == NULL) printf("%s%s\n", END_SIGNAL, task->name);
        else printf("%s%s\n", T_SIGNAL, task->name);
        task = task->next;
    }

    // 遍历FS_t特定深度
    // 递归遍历子节点
    if (level < depth){
        FS_t child_tmp = node->child_next;
        while (child_tmp != NULL){
            ShowTab(level);
            if (child_tmp->level_next == NULL) {
                printf("%s%s\n", END_SIGNAL, child_tmp->path);
                return;
            }
            else printf("%s%s\n", T_SIGNAL, child_tmp->path);

            dfs(child_tmp, level + 1, depth);
            child_tmp = child_tmp->level_next;
        }
    }
}

/***
 * @brief 深度优先搜索算法
 * @param node 文件系统节点
 * @param depth 需要遍历深度
 * */
void DFS(FS_t node, int depth){
    dfs(node, 0, depth);
}
