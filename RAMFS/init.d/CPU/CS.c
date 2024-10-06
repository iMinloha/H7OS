#include <string.h>
#include "CS.h"
#include "memctl.h"
#include "quadspi.h"

CS_t cs_head = NULL;

void CS_push(char *save_str, int len){
    char data[128];
    memcpy(data, save_str, len);
    if (cs_head == NULL){
        cs_head = (CS_t) kernel_alloc(CS_SIZE);
        cs_head->next = NULL;
        memcpy(cs_head->data, data, len);
    }else{
        CS_t tmp = cs_head;
        while (tmp->next != NULL) tmp = tmp->next;
        tmp->next = (CS_t) kernel_alloc(CS_SIZE);
        tmp->next->next = NULL;
        memcpy(tmp->next->data, data, len);
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