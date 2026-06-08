#include "Core/DrT.h"
#include "memctl.h"
#include <string.h>

extern FS_t currentFS;

void addThread(Task_t task) {
    FS_t node = getFSChild(RAM_FS, "proc");
    if (node == NULL) return;
    Task_t p = node->tasklist;
    if (p == NULL) {
        node->tasklist = task;
        return;
    }
    else {
        while (p->next != NULL) p = p->next;
        p->next = task;
    }
}


Task_t getThread(char* name) {
    FS_t node = getFSChild(RAM_FS, "proc");
    if (node == NULL) return NULL;
    Task_t p = node->tasklist;
    while (p != NULL) {
        if (strcmp(p->name, name) == 0) return p;
        p = p->next;
    }
    return NULL;
}


Task_t getTaskList() {
    FS_t node = getFSChild(RAM_FS, "proc");
    if (node == NULL) return NULL;
    return node->tasklist;
}


Task_t getTaskByHandle(osThreadId handle) {
    FS_t node = getFSChild(RAM_FS, "proc");
    if (node == NULL) return NULL;
    Task_t p = node->tasklist;
    while (p != NULL) {
        if (p->handle == handle) return p;
        p = p->next;
    }
    return NULL;
}


Task_t getThreadByPID(uint8_t pid) {
    FS_t node = getFSChild(RAM_FS, "proc");
    if (node == NULL) return NULL;
    Task_t p = node->tasklist;
    while (p != NULL) {
        if (p->PID == pid) return p;
        p = p->next;
    }
    return NULL;
}

Task_t loadTask(char* path) {
    FS_t node;
    if (path[0] == '/') node = RAM_FS;
    else node = currentFS;

    if (strcmp(path, "/") == 0) return NULL;

    else {
        if (path[0] == '/') path++;
        char* token = strtok(path, "/");
        while (token != NULL) {
            FS_t tmp_node = getFSChild(node, token);
            if (tmp_node == NULL) break;
            token = strtok(NULL, "/");
            node = tmp_node;
        }

        // token不能包含任何的/
        if (strcmp(token, strtok(token, "/")) != 0) return NULL;
        else {
            Task_t p = node->tasklist;
            while (p != NULL) {
                if (strcmp(p->name, token) == 0) return p;
                p = p->next;
            }
        }
    }

    return NULL;
}
