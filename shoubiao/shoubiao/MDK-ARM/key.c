#include "key.h"
#include "main.h"

// 按键事件队列
osMessageQueueId_t keyEventQueue;

// 按键长按阈值 (ms)
#define LONG_PRESS_THRESHOLD 2000

// 按键状态机
typedef struct {
    KeyState current_state;
    uint32_t press_start_time;
    uint8_t is_debouncing;
    uint32_t debounce_start_time;
} KeyFSM;

KeyFSM keys[4] = {0};

/**
  * @brief  按键初始化
  */
void KEY_Init(void)
{
    // 按键外部中断已经在CubeMX中配置，这里初始化队列
    keyEventQueue = osMessageQueueNew(10, sizeof(KeyEvent), NULL);
    
    // 初始化状态机
    for(int i = 0; i < 4; i++)
    {
        keys[i].current_state = KEY_RELEASED;
        keys[i].press_start_time = 0;
        keys[i].is_debouncing = 0;
        keys[i].debounce_start_time = 0;
    }
}

/**
  * @brief  按键扫描函数
  */
uint8_t KEY_Scan(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    static uint8_t key_up = 1;
    
    if(key_up && (HAL_GPIO_ReadPin(GPIOx, GPIO_Pin) == GPIO_PIN_SET))
    {
        osDelay(20); // 消抖
        key_up = 0;
        if(HAL_GPIO_ReadPin(GPIOx, GPIO_Pin) == GPIO_PIN_SET)
            return 1;
    }
    else if(HAL_GPIO_ReadPin(GPIOx, GPIO_Pin) == GPIO_PIN_RESET)
    {
        key_up = 1;
    }
    
    return 0;
}

/**
  * @brief  按键任务处理函数
  */
void KEY_Task(void *argument)
{
    TickType_t last_wake_time = osKernelGetTickCount();
    const TickType_t period = 20; // 20ms扫描周期
    
    for(;;)
    {
        // 扫描所有按键
        uint8_t key_states[4] = {
            HAL_GPIO_ReadPin(KEY_RIGHT_PORT, KEY_RIGHT_PIN),
            HAL_GPIO_ReadPin(KEY_LEFT_PORT, KEY_LEFT_PIN),
            HAL_GPIO_ReadPin(KEY_ENTER_PORT, KEY_ENTER_PIN),
            HAL_GPIO_ReadPin(KEY_EXIT_PORT, KEY_EXIT_PIN)
        };
        
        // 处理每个按键的状态机
        for(int i = 0; i < 4; i++)
        {
            switch(keys[i].current_state)
            {
                case KEY_RELEASED:
                    if(key_states[i] == GPIO_PIN_SET) // 按键按下（上拉模式下按下为高电平）
                    {
                        keys[i].is_debouncing = 1;
                        keys[i].debounce_start_time = osKernelGetTickCount();
                        keys[i].current_state = KEY_PRESSED;
                        keys[i].press_start_time = osKernelGetTickCount();
                        
                        // 发送按键按下事件
                        KeyEvent event = {i, KEY_PRESSED, osKernelGetTickCount()};
                        osMessageQueuePut(keyEventQueue, &event, 0, 0);
                    }
                    break;
                    
                case KEY_PRESSED:
                    if(key_states[i] == GPIO_PIN_RESET) // 按键释放
                    {
                        keys[i].current_state = KEY_RELEASED;
                    }
                    else // 按键仍然按住
                    {
                        uint32_t press_duration = osKernelGetTickCount() - keys[i].press_start_time;
                        if(press_duration > pdMS_TO_TICKS(LONG_PRESS_THRESHOLD))
                        {
                            keys[i].current_state = KEY_LONG_PRESSED;
                            
                            // 发送长按事件
                            KeyEvent event = {i, KEY_LONG_PRESSED, osKernelGetTickCount()};
                            osMessageQueuePut(keyEventQueue, &event, 0, 0);
                        }
                    }
                    break;
                    
                case KEY_LONG_PRESSED:
                    if(key_states[i] == GPIO_PIN_RESET) // 按键释放
                    {
                        keys[i].current_state = KEY_RELEASED;
                    }
                    break;
            }
        }
        
        osDelayUntil(last_wake_time + period);
        last_wake_time = osKernelGetTickCount();
    }
}

/**
  * @brief  获取按键事件
  */
KeyEvent KEY_GetEvent(void)
{
    KeyEvent event = {0, KEY_RELEASED, 0};
    
    if(osMessageQueueGetCount(keyEventQueue) > 0)
    {
        osMessageQueueGet(keyEventQueue, &event, NULL, 0);
    }
    
    return event;
}

/**
  * @brief  清除所有按键事件
  */
void KEY_ClearEvents(void)
{
    KeyEvent event;
    while(osMessageQueueGet(keyEventQueue, &event, NULL, 0) == osOK)
    {
        // 清空队列
    }
}