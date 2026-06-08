#ifndef H7OS_LCD_DEFINE_H
#define H7OS_LCD_DEFINE_H

#include <stdint-gcc.h>

#include "ff.h"

typedef struct _pFont{
    const uint8_t *pTable;
    uint16_t Width;
    uint16_t Height;
    uint16_t Sizes;
    uint16_t Table_Rows;
} pFONT;

typedef struct _pChineseFont {
    uint16_t Width;
    uint16_t Height;
    uint16_t Sizes;
    const TCHAR * FontPath;
} pChineseFont;

extern	pChineseFont CH_Font12 ;
extern	pChineseFont CH_Font16 ;
extern	pChineseFont CH_Font24 ;
extern	pChineseFont CH_Font32 ;

#define  FontPath_CH_Font12  "0:GB2312_12.FON"
#define  FontPath_CH_Font16  "0:GB2312_16.FON"
#define  FontPath_CH_Font24  "0:GB2312_24.FON"
#define  FontPath_CH_Font32  "0:GB2312_32.FON"

extern pFONT Font32;
extern pFONT Font24;
extern pFONT Font20;
extern pFONT Font16;
extern pFONT Font12;

#endif //H7OS_LCD_DEFINE_H
