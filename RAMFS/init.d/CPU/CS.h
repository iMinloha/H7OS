#ifndef H7OS_CS_H
#define H7OS_CS_H

// CS_t是一个字符结构体，用于保存每次的指令. 例如：
// CS_t cs = {"mkdir /tmp", 9};
typedef struct cs {
    // data是指令的字符串
    char data[128];
    // next是指向下一个CS_t的指针
    struct cs *next;
}* CS_t;

#define CS_SIZE sizeof (struct cs)

void CS_push(char *save_str);

void CS_list();

void CS_save();


#endif
