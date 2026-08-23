#ifndef __OLED_H
#define __OLED_H

#include "main.h"
#include "i2c.h"
#include <string.h>

// OLED I2C地址
#define OLED_ADDRESS 0x78

// OLED控制命令
#define OLED_CMD  0x00
#define OLED_DATA 0x40

// 屏幕尺寸
#define OLED_WIDTH  128
#define OLED_HEIGHT 64
#define OLED_PAGES  8

// 字体大小定义
#define FONT_SIZE_6x8   1
#define FONT_SIZE_8x16  2
#define FONT_SIZE_16x16 3

// 颜色定义
#define BLACK 0
#define WHITE 1

// 函数声明
void OLED_Init(void);
void OLED_Clear(void);
void OLED_Refresh(void);
void OLED_SetCursor(uint8_t page, uint8_t col);
void OLED_ShowChar(uint8_t x, uint8_t y, char ch, uint8_t size);
void OLED_ShowString(uint8_t x, uint8_t y, char *str, uint8_t size);
void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size);
void OLED_ShowFloatNum(uint8_t x, uint8_t y, float num, uint8_t int_len, uint8_t size);
void OLED_ShowChinese(uint8_t x, uint8_t y, uint8_t index);
void OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t color);
void OLED_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t color);
void OLED_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t color);
void OLED_Fill(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t color);
void OLED_ShowBMP(uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint8_t *bmp);

// 全局显示缓冲区
extern uint8_t OLED_GRAM[128][8];

#endif
