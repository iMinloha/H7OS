#ifndef H7OS_CS_H
#define H7OS_CS_H

#define QSPI_Page_HaveCS 0xbb
#define QSPI_Page_None 0x00

// CS_t是一个字符结构体，用于保存每次的指令. 例如：
// CS_t cs = {"mkdir /tmp", 9};
typedef struct cs {
    // data是指令的字符串
    // 不是128的理由很简单，留出1位当标志
    char data[126]; // 126 + 1 = 127 (1位是\0)
    // next是指向下一个CS_t的指针
    struct cs *next;
}* CS_t;

// 指令块大小
#define CS_SIZE sizeof (struct cs)
// NULL的反义
#define Normal ((void *)1)

// 添加指令保存
void CS_push(char *save_str);

// 清空保存
void CS_clean();

// 输出展示
void CS_list();

// 保存
void CS_save();

// 加载
void CS_load();

// 执行
void CS_Run();

#endif
