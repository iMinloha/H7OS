#include "timer.h"
#include "rtc.h"
#include "rng.h"

RTC_TimeTypeDef Time_Struct;
RTC_DateTypeDef Date_Struct;

/***
 * 更新时间
 */
void UpdateTime(){
    HAL_RTC_GetTime(&hrtc,&Time_Struct,RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc,&Date_Struct,RTC_FORMAT_BIN);
}

/***
 * 获取当前秒
 * @return 当前秒
 */
uint8_t getSecond(){
    return Time_Struct.Seconds;
}

/***
 * 获取当前分钟
 * @return 当前分钟
 */
uint8_t getMinute(){
    return Time_Struct.Minutes + Correct_Minute;
}

/***
 * 获取当前小时
 * @return 当前小时
 */
uint8_t getHour(){
    return Time_Struct.Hours + Correct_Hour;
}

/***
 * 获取当前日期
 * @return 当前日期
 */
uint8_t getDay(){
    return Date_Struct.Date + Correct_Day;
}

/***
 * 获取当前月份
 * @return 当前月份
 */
uint8_t getMonth(){
    return Date_Struct.Month + Correct_Month;
}

/***
 * 获取当前年份
 * @return 当前年份
 */
uint16_t getYear(){
    return Date_Struct.Year + Correct_Year;
}

/***
 * 获取随机数
 * @return 随机数
 */
uint32_t Random(){
    uint32_t *random = NULL;
    HAL_RNG_GenerateRandomNumber(&hrng, random);
    return *random;
}
