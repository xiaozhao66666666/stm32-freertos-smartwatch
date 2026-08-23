#ifndef __FONT_H
#define __FONT_H

#include <stdint.h>

// ASCII 6x8 字体
extern const uint8_t ASCII_6x8[][6];

// ASCII 8x16 字体
extern const uint8_t ASCII_8x16[][16];

// 中文 16x16 字体 (示例：可以添加常用的汉字)
extern const uint8_t Chinese_16x16[][32];

// 常用图标或符号
extern const uint8_t Icon_16x16[][32];

#endif