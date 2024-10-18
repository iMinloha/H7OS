#include <string.h>
#include "CS.h"
#include "memctl.h"
#include "quadspi.h"
#include "init.d/DrT/DrT.h"

CS_t cs_head = NULL;

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
    char *buf_tmp = kernel_alloc(strlen(save_str));
    memcpy(buf_tmp, save_str, strlen(save_str));

    CS_t tmp = cs_head;
    while (tmp != NULL){
        int len = commandFound(buf_tmp, tmp->data);
        if (len > 0) return NULL;   // 无操作
        else if (len < 0) return tmp;   // 替换原本的内容
        tmp = tmp->next;
    }
    kernel_free(buf_tmp);
    return Normal;
}

// 删除重复指令
void CS_removeSameCommand(CS_t response, char *buf){
    char *data = kernel_alloc(128);
    memcpy(data, buf, 128);
    CS_t tmp = cs_head;
    CS_t pre = NULL;
    while (tmp != NULL){
        if (strcmp(tmp->data, data) == 0 && tmp != response){
            if (pre == NULL) {
                kernel_free(tmp);
                break;
            } else{
                pre->next = tmp->next;
                kernel_free(tmp);
                break;
            }
        }
        pre = tmp;
        tmp = tmp->next;
    }
    kernel_free(data);
}

// 推入新的指令缓存
void CS_push(char *buf){
    char *data = kernel_alloc(128);
    memcpy(data, buf, 128);

    CS_t response = CS_replaceSubCommand(data);

    if (response == NULL) return;
    else if (response == Normal){
        if (cs_head == NULL) {
            cs_head = (CS_t) kernel_alloc(CS_SIZE);
            cs_head->next = NULL;
            memcpy(cs_head->data, data, W25Qxx_PageSize);
        } else {
            CS_t tmp = cs_head;
            while (tmp->next != NULL) tmp = tmp->next;
            tmp->next = (CS_t) kernel_alloc(CS_SIZE);
            tmp->next->next = NULL;
            memcpy(cs_head->data, data, W25Qxx_PageSize);
        }
    }else{
        CS_removeSameCommand(response, data);
        memcpy(response->data, data, W25Qxx_PageSize);
    }
    kernel_free(data);
}

// 朴素插值
// 无查询环节的插值
void CS_normal_append(char *buf){
    char *data = kernel_alloc(128);
    memcpy(data, buf, 128);
    // 插值
    CS_t tmp_head = cs_head;
    if (tmp_head == NULL) {
        tmp_head = (CS_t) kernel_alloc(CS_SIZE);
        tmp_head->next = NULL;
        memcpy(tmp_head->data, data, W25Qxx_PageSize);
    } else {
        CS_t tmp = cs_head;
        while (tmp->next != NULL) tmp = tmp->next;
        tmp->next = (CS_t) kernel_alloc(CS_SIZE);
        tmp->next->next = NULL;
        memcpy(cs_head->data, data, W25Qxx_PageSize);
    }
}

// 输出指令缓存内容
void CS_list(){
    CS_t tmp = cs_head;
    while (tmp != NULL){
        printf("%s\n", tmp->data);
        tmp = tmp->next;
    }
}

void CS_Run(){
    CS_t tmp = cs_head;
    while (tmp != NULL){
        execCMD(tmp->data);
        tmp = tmp->next;
    }
}

// 历史指令清除
void CS_clean(){
    CS_t tmp = cs_head;
    while (tmp != NULL){
        CS_t next = tmp->next;
        kernel_free(tmp);
        tmp = next;
    }
    cs_head = NULL;
}

// 保存指令缓存
// save周期非常长, 一定要缓速保存
void CS_save(){
    QSPI_W25Qxx_ChipErase();
    uint8_t i = 0;
    CS_t tmp = cs_head;
    while (tmp != NULL){
        uint8_t *p_data = (uint8_t *) kernel_alloc(128);
        memset(p_data, 0, 128);

        p_data[0] = QSPI_Page_HaveCS;
        memcpy((char*) p_data + 1, tmp->data, W25Qxx_PageSize - 1);

        QSPI_W25Qxx_WriteBuffer(p_data, i * W25Qxx_PageSize, 128);

        kernel_free(p_data);
        tmp = tmp->next;
        i++;
    }
}

// 加载CS缓存指令
void CS_load(){
    uint8_t *p_data = (uint8_t *) kernel_alloc(128);
    memset(p_data, 0, 128);
    QSPI_W25Qxx_ReadBuffer(p_data, 0, 128);
    if (p_data[0] != QSPI_Page_HaveCS) return;

    if (p_data[0] == QSPI_Page_HaveCS){
        char *buf = kernel_alloc(128);
        strcpy(buf, (char*) p_data + 1);
        CS_push(buf);
        execCMD(buf);
        kernel_free(buf);
    }

    int i = 1;
    while (p_data[0] == QSPI_Page_HaveCS){
        memset(p_data, 0, 128);
        QSPI_W25Qxx_ReadBuffer(p_data, i * W25Qxx_PageSize, 128);
        if (p_data[0] == QSPI_Page_HaveCS){
            char *buf = kernel_alloc(128);
            memcpy(buf, (char*) p_data + 1, W25Qxx_PageSize - 1);
            kernel_free(buf);
        }
        i++;
    }
    kernel_free(p_data);
}