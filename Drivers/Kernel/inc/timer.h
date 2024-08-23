#ifndef H7OS_TIMER_H
#define H7OS_TIMER_H

#include <stdint-gcc.h>


// 开机时间, 需要根据实际情况修改
#define Correct_Year 2024
#define Correct_Month 8 - 1
#define Correct_Day 14 - 1
#define Correct_Hour 22
#define Correct_Minute 9

/***
 * 获取当前秒
 * @return 当前秒
 */
uint8_t getSecond();

/***
 * 获取当前分钟
 * @return 当前分钟
 */
uint8_t getMinute();

/***
 * 获取当前小时
 * @return 当前小时
 */
uint8_t getHour();

/***
 * 获取当前日期
 * @return 当前日期
 */
uint8_t getDay();

/***
 * 获取当前月份
 * @return 当前月份
 */
uint8_t getMonth();

/***
 * 获取当前年份
 * @return 当前年份
 */
uint16_t getYear();

/***
 * 更新时间
 */
void UpdateTime();

/***
 * 获取随机数
 * @return 随机数
 */
uint32_t Random();

#endif //H7OS_TIMER_H
