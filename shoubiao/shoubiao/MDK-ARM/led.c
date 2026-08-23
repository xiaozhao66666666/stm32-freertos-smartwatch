#include "led.h"
#include "cmsis_os.h"

// LED当前模式
static LED_Mode led_mode = LED_MODE_OFF;
static uint32_t led_blink_counter = 0;

/**
  * @brief  LED初始化
  */
void LED_Init(void)
{
    // GPIO已经在CubeMX中配置为输出
    LED_Off();
}

/**
  * @brief  打开LED
  */
void LED_On(void)
{
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, LED_ON);
}

/**
  * @brief  关闭LED
  */
void LED_Off(void)
{
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, LED_OFF);
}

/**
  * @brief  切换LED状态
  */
void LED_Toggle(void)
{
    HAL_GPIO_TogglePin(LED_PORT, LED_PIN);
}

/**
  * @brief  设置LED模式
  */
void LED_SetMode(LED_Mode mode)
{
    led_mode = mode;
    led_blink_counter = 0;
    
    if(mode == LED_MODE_OFF)
        LED_Off();
    else if(mode == LED_MODE_ON)
        LED_On();
}

/**
  * @brief  更新LED状态（需要在定时任务中调用）
  */
void LED_Update(void)
{
    static uint32_t last_update_time = 0;
    uint32_t current_time = osKernelGetTickCount();
    
    if(current_time - last_update_time < 100) // 每100ms更新一次
        return;
    
    last_update_time = current_time;
    
    switch(led_mode)
    {
        case LED_MODE_BLINK_SLOW:
            led_blink_counter++;
            if(led_blink_counter % 10 == 0) // 1秒周期
                LED_Toggle();
            break;
            
        case LED_MODE_BLINK_FAST:
            led_blink_counter++;
            if(led_blink_counter % 2 == 0) // 200ms周期
                LED_Toggle();
            break;
            
        case LED_MODE_BREATH:
            // 呼吸灯效果（需要PWM实现，这里简化为闪烁）
            led_blink_counter++;
            if(led_blink_counter % 5 == 0) // 500ms周期
                LED_Toggle();
            break;
            
        default:
            break;
    }
}