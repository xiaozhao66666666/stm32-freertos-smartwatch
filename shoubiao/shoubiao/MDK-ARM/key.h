#ifndef __KEY_H
#define __KEY_H

#include "main.h"
#include "cmsis_os.h"

// 按键引脚定义
#define KEY_RIGHT_PIN  GPIO_PIN_11
#define KEY_RIGHT_PORT GPIOB

#define KEY_LEFT_PIN   GPIO_PIN_10
#define KEY_LEFT_PORT  GPIOB

#define KEY_ENTER_PIN  GPIO_PIN_1
#define KEY_ENTER_PORT GPIOB

#define KEY_EXIT_PIN   GPIO_PIN_0
#define KEY_EXIT_PORT  GPIOB

// 按键状态
typedef enum {
    KEY_RELEASED = 0,
    KEY_PRESSED,
    KEY_LONG_PRESSED
} KeyState;

// 按键事件队列
extern osMessageQueueId_t keyEventQueue;

// 按键事件结构
typedef struct {
    uint8_t key_id;
    KeyState state;
    uint32_t press_time;
} KeyEvent;

// 按键ID
typedef enum {
    KEY_ID_RIGHT = 0,
    KEY_ID_LEFT,
    KEY_ID_ENTER,
    KEY_ID_EXIT
} KeyID;

// 函数声明
void KEY_Init(void);
uint8_t KEY_Scan(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
void KEY_Task(void *argument);
KeyEvent KEY_GetEvent(void);
void KEY_ClearEvents(void);

#endif