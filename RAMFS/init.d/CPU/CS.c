#include <string.h>
#include "CS.h"
#include "memctl.h"
#include "quadspi.h"

CS_t cs_head = NULL;
#define Normal ((void *)1)

// 头部开始的子串匹配,去除深度创建重复文件夹
int commandFound(char* str1, char* str2){
    char* tmp1 = str1;
    char* tmp2 = str2;
    int len = 0;
    while (tmp1 != NULL && tmp2 != NULL){
        if (tmp1[0] != tmp2[0]) break;
        tmp1++;
        tmp2++;
        len++;
    }

    if (len == strlen(str1) && len < strlen(str2)) return len;
    if (len == strlen(str2) && len < strlen(str1)) return -len;
    return 0;
}

// 比较新加入的字符串是否需要操作
/***
 * @brief 串判断
 * @param save_str 保存的字符串
 * @return NULL 无操作
 * @return Normal 按照正常操作
 * @return CS_t 找到子指令，返回这个子指令的位置
 * */
CS_t CS_replaceSubCommand(char *save_str){
    CS_t tmp = cs_head;
    while (tmp != NULL){
        int len = commandFound(save_str, tmp->data);
        if (len > 0) return NULL;   // 无操作
        else if (len < 0) return tmp;   // 替换原本的内容
        tmp = tmp->next;
    }
    return Normal;
}

// 删除重复指令
void CS_removeSameCommand(char *buf){
    char *data = kernel_alloc(128);
    memcpy(data, buf, 128);
    CS_t tmp = cs_head;
    CS_t pre = NULL;
    while (tmp != NULL){
        if (strcmp(tmp->data, data) == 0){
            pre->next = tmp->next;
            kernel_free(tmp);
            break;
        }
        pre = tmp;
        tmp = tmp->next;
    }
}

void CS_push(char *save_str){
    char data[128];
    int len = strlen(save_str);
    memcpy(data, save_str, len);

    CS_t response = CS_replaceSubCommand(data);

    if (response == NULL){
        // 重复内容无需操作
        return;
    }else if (response == Normal){
        if (cs_head == NULL) {
            cs_head = (CS_t) kernel_alloc(CS_SIZE);
            cs_head->next = NULL;
            memcpy(cs_head->data, data, len);
        } else {
            CS_t tmp = cs_head;
            while (tmp->next != NULL) tmp = tmp->next;
            tmp->next = (CS_t) kernel_alloc(CS_SIZE);
            tmp->next->next = NULL;
            memcpy(tmp->next->data, data, len);
        }
    }else{
        CS_removeSameCommand(data);
        memcpy(response->data, data, len);
    }
}

void CS_list(){
    CS_t tmp = cs_head;
    while (tmp != NULL){
        printf("%s\n", tmp->data);
        tmp = tmp->next;
    }
}

void CS_save(){
    int i = 0;
    CS_t tmp = cs_head;
    while (tmp != NULL){
        QSPI_W25Qxx_WriteBuffer((uint8_t *)tmp->data, i * W25Qxx_PageSize, 128);
        tmp = tmp->next;
    }
}