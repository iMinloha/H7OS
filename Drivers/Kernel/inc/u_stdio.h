#ifndef H7OS_U_STDIO_H
#define H7OS_U_STDIO_H

#include <stdint.h>

// 布尔值
#define TRUE 1
#define FALSE 0

// 字符串结束符
#define EOL '\n'
#define EOLR '\r'
#define EOLN '\0'

#define ThreadLock    __disable_irq()
#define ThreadUnlock  __enable_irq()

// 格式字符
#define _INTSIZEOF(n)  ((sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1))
#define va_start(ap,v) (ap = (va_list)&v + _INTSIZEOF(v))
#define va_arg(ap,t)   (*(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)))
#define va_end(ap)     ( ap = (va_list)0 )

typedef char * va_list;

#define _OS_WEAK __attribute__((weak))
// 对象类型
#define type(x) typeof(x)

/**
 * @brief 输出格式化字符串到串口
 * @param fmt: 格式化字符串
 * @param ...: 可变参数
 * @retval 0
 * */
int u_print(const char *fmt, ...);

/**
 * @brief 输出字符到串口
 * @param fmt: 格式化字符串
 * @param ap: 可变参数列表
 * */
static int v_printf(const char *fmt, va_list ap);

/**
 * @brief 输出字符到串口
 * @param c: 字符
 * */
void put_char(char c);

/**
 * @brief 输出数字到串口
 * @param num: 数字
 * */
void put_num(int num, int base, int sign);

/**
 * @brief 输出浮点数到串口
 * @param num: 浮点数
 * */
void put_float(float num, int base, int sign);

/**
 * @brief 输出双精度浮点数到串口
 * @param num: 双精度浮点数
 * */
void put_double(double num, int base, int sign);

/**
 * @brief 输出大数字到串口
 * @param num: 大数字
 * */
void put_huge_num(uint32_t num, int base, int sign);

/**
 * @brief 输出地址到串口
 * @param num: 地址
 * */
void put_address_num(uint32_t num, int base, int sign);

/**
 * @brief 输出字符串到串口
 * @param str: 字符串
 * */
void put_s(char *str);

/**
 * @brief get the length of a string
 * @param str: string
 * @retval length of the string
 * */
int strlen(char *str);

/**
 * @brief split a string by a delimiter
 * @param str: string
 * @param delim: delimiter
 * @retval the first token
 * */
char* strtok(char *str, char *delim);

/**
 * @brief split a string by a delimiter
 * @param str: string
 * @param delim: delimiter
 * @param outlist: output list
 * @retval number of elements in the output list
 * */
int strspilt(char *str, char *delim, char *outlist[]);

/**
 * @brief output formatted string to a buffer
 * @param out: output buffer
 * @param fmt: format string
 * @param ...: variable arguments
 * */
void u_sprintf(char *out, const char *fmt, ...);

/**
 * @brief copy a string
 * @param dest: destination string
 * @param src: source string
 * */
void strcopy(char *dest, char *src);

/**
 * @brief concatenate two strings
 * @param dest: destination string
 * @param src: source string
 * */
void strconcat(char *dest, char *src);

/**
 * @brief compare two strings
 * @param str1: string 1
 * @param str2: string 2
 * @retval 0 if equal, 1 if not equal
 * */
int strcmp(char *str1, char *str2);


#define LED_ON HAL_GPIO_WritePin(GPIOH, GPIO_PIN_7, GPIO_PIN_RESET)

#define LED_OFF HAL_GPIO_WritePin(GPIOH, GPIO_PIN_7, GPIO_PIN_SET)

#endif
