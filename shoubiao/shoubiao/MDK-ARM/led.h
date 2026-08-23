#ifndef __LED_H
#define __LED_H

#include "main.h"

// LED引脚定义
#define LED_PIN  GPIO_PIN_13
#define LED_PORT GPIOC

// LED状态
#define LED_ON   GPIO_PIN_RESET
#define LED_OFF  GPIO_PIN_SET

// LED模式
typedef enum {
    LED_MODE_OFF = 0,
    LED_MODE_ON,
    LED_MODE_BLINK_SLOW,
    LED_MODE_BLINK_FAST,
    LED_MODE_BREATH
} LED_Mode;

// 函数声明
void LED_Init(void);
void LED_On(void);
void LED_Off(void);
void LED_Toggle(void);
void LED_SetMode(LED_Mode mode);
void LED_Update(void);

#endif