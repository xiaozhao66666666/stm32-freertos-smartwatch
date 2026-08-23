/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "oled.h"
#include "key.h"
#include "led.h"
#include "driver_dht11.h"
#include "driver_passive_buzzer.h"
// <<< 新增：包含所有需要的头文件
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
// <<< 新增：定义屏幕枚举类型
typedef enum {
    SCREEN_CLOCK,      // 时钟界面
    SCREEN_CALENDAR,   // 日历界面
    SCREEN_TEMP_HUM,   // 温湿度界面
    SCREEN_FLASHLIGHT, // 手电筒界面
    SCREEN_SETTINGS,   // 设置界面
    SCREEN_MAX
} ScreenType_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
// <<< 新增：全局变量定义
ScreenType_t CurrentScreen = SCREEN_CLOCK; // 当前显示的屏幕
float gTemperature = 0, gHumidity = 0;     // 温湿度全局变量
uint8_t gLedStatus = 0;                    // LED状态
uint8_t gBuzzerStatus = 0;                 // 蜂鸣器状态
/* USER CODE END Variables */

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Definitions for MenuTask */
osThreadId_t MenuTaskHandle;
const osThreadAttr_t MenuTask_attributes = {
  .name = "MenuTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

// <<< 新增：其他任务句柄和属性定义
/* Definitions for DisplayTask */
osThreadId_t DisplayTaskHandle;
const osThreadAttr_t DisplayTask_attributes = {
  .name = "DisplayTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Definitions for ClockTask */
osThreadId_t ClockTaskHandle;
const osThreadAttr_t ClockTask_attributes = {
  .name = "ClockTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Definitions for DHT11Task */
osThreadId_t DHT11TaskHandle;
const osThreadAttr_t DHT11Task_attributes = {
  .name = "DHT11Task",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal,
};

/* Definitions for KeyTask */
osThreadId_t KeyTaskHandle;
const osThreadAttr_t KeyTask_attributes = {
  .name = "KeyTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};

/* Definitions for SystemMonitorTask */
osThreadId_t SystemMonitorTaskHandle;
const osThreadAttr_t SystemMonitorTask_attributes = {
  .name = "SystemMonitorTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
// <<< 新增：任务函数声明
void DisplayTaskFunc(void *argument);
void ClockTaskFunc(void *argument);
void SystemMonitorTaskFunc(void *argument);
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartTask02(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of MenuTask */
  MenuTaskHandle = osThreadNew(StartTask02, NULL, &MenuTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  // <<< 新增：创建所有任务
  /* creation of DisplayTask */
  DisplayTaskHandle = osThreadNew(DisplayTaskFunc, NULL, &DisplayTask_attributes);
  
  /* creation of ClockTask */
  ClockTaskHandle = osThreadNew(ClockTaskFunc, NULL, &ClockTask_attributes);
  
  /* creation of DHT11Task */
  DHT11TaskHandle = osThreadNew(DHT11_Task, NULL, &DHT11Task_attributes);
  
  /* creation of KeyTask */
  KeyTaskHandle = osThreadNew(KEY_Task, NULL, &KeyTask_attributes);
  
  /* creation of SystemMonitorTask */
  SystemMonitorTaskHandle = osThreadNew(SystemMonitorTaskFunc, NULL, &SystemMonitorTask_attributes);
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1000);  // <<< 修改：延长延时时间
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartTask02 */
/**
* @brief Function implementing the MenuTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask02 */
void StartTask02(void *argument)
{
  /* USER CODE BEGIN StartTask02 */
  // <<< 修改：菜单任务实现
  /* 初始化外设 */
  KEY_Init();
  LED_Init();
  BUZZER_Init();
  
  /* 开机提示音 */
  BUZZER_Beep(200);
  LED_SetMode(LED_MODE_BLINK_FAST);
  osDelay(500);
  LED_SetMode(LED_MODE_OFF);
  
  for(;;)
  {
    // 处理按键事件
    KeyEvent event = KEY_GetEvent();
    if(event.state != KEY_RELEASED)
    {
      switch(event.key_id)
      {
        case KEY_ID_RIGHT:  // 右键：下一个屏幕
          CurrentScreen = (CurrentScreen + 1) % SCREEN_MAX;
          BUZZER_Beep(50);  // 按键提示音
          break;
          
        case KEY_ID_LEFT:   // 左键：上一个屏幕
          if(CurrentScreen == 0)
            CurrentScreen = SCREEN_MAX - 1;
          else
            CurrentScreen--;
          BUZZER_Beep(50);  // 按键提示音
          break;
          
        case KEY_ID_ENTER:  // 确认键：进入/确认
          if(CurrentScreen == SCREEN_FLASHLIGHT)
          {
            gLedStatus = !gLedStatus;
            if(gLedStatus)
              LED_SetMode(LED_MODE_ON);
            else
              LED_SetMode(LED_MODE_OFF);
          }
          else if(CurrentScreen == SCREEN_SETTINGS)
          {
            gBuzzerStatus = !gBuzzerStatus;
            BUZZER_Beep(100);
          }
          break;
          
        case KEY_ID_EXIT:   // 退出键：返回
          // 可以添加返回上级菜单的逻辑
          break;
      }
    }
    
    // 更新LED和蜂鸣器状态
    LED_Update();
    BUZZER_Update();
    
    osDelay(20);  // 20ms周期
  }
  /* USER CODE END StartTask02 */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
// <<< 新增：所有新任务函数的实现

/**
  * @brief  显示任务函数
  * @param  argument: Not used
  * @retval None
  */
void DisplayTaskFunc(void *argument)
{
  /* 初始化OLED */
  OLED_Init();
  OLED_Clear();
  
  /* 显示开机画面 */
  OLED_ShowString(30, 0, "SMART WATCH", FONT_SIZE_6x8);
  OLED_ShowString(20, 2, "Initializing...", FONT_SIZE_6x8);
  OLED_Refresh();
  osDelay(1000);
  
  for(;;)
  {
    OLED_Clear();
    
    /* 根据当前屏幕显示不同内容 */
    switch(CurrentScreen)
    {
      case SCREEN_CLOCK:      // 时钟界面
        OLED_ShowString(40, 0, "CLOCK", FONT_SIZE_6x8);
        OLED_ShowString(20, 2, "Time: 12:30:45", FONT_SIZE_6x8);
        OLED_ShowString(20, 4, "Date: 2024-01-01", FONT_SIZE_6x8);
        break;
        
      case SCREEN_CALENDAR:   // 日历界面
        OLED_ShowString(35, 0, "CALENDAR", FONT_SIZE_6x8);
        OLED_ShowString(20, 2, "2024年1月1日", FONT_SIZE_6x8);
        OLED_ShowString(20, 4, "星期一", FONT_SIZE_6x8);
        OLED_ShowString(20, 6, "农历冬月廿一", FONT_SIZE_6x8);
        break;
        
      case SCREEN_TEMP_HUM:   // 温湿度界面
        OLED_ShowString(30, 0, "ENVIRONMENT", FONT_SIZE_6x8);
        
        // 显示温度图标
        OLED_ShowString(10, 2, "Temp:", FONT_SIZE_6x8);
        OLED_ShowFloatNum(50, 2, gTemperature, 2, FONT_SIZE_6x8);
        OLED_ShowString(90, 2, "C", FONT_SIZE_6x8);
        
        // 显示湿度图标
        OLED_ShowString(10, 4, "Hum:", FONT_SIZE_6x8);
        OLED_ShowFloatNum(50, 4, gHumidity, 2, FONT_SIZE_6x8);
        OLED_ShowString(90, 4, "%", FONT_SIZE_6x8);
        break;
        
      case SCREEN_FLASHLIGHT: // 手电筒界面
        OLED_ShowString(30, 0, "FLASHLIGHT", FONT_SIZE_6x8);
        OLED_ShowString(15, 3, gLedStatus ? "ON" : "OFF", FONT_SIZE_8x16);
        OLED_ShowString(30, 5, "Press ENTER", FONT_SIZE_6x8);
        OLED_ShowString(30, 6, "to toggle", FONT_SIZE_6x8);
        break;
        
      case SCREEN_SETTINGS:   // 设置界面
        OLED_ShowString(35, 0, "SETTINGS", FONT_SIZE_6x8);
        OLED_ShowString(10, 2, "Buzzer:", FONT_SIZE_6x8);
        OLED_ShowString(60, 2, gBuzzerStatus ? "ON " : "OFF", FONT_SIZE_6x8);
        OLED_ShowString(10, 4, "Brightness:", FONT_SIZE_6x8);
        OLED_ShowString(80, 4, "80%", FONT_SIZE_6x8);
        break;
        
      default:
        break;
    }
    
    /* 显示底部导航栏 */
    OLED_DrawLine(0, 56, 127, 56, WHITE);
    OLED_ShowString(5, 7, "<", FONT_SIZE_6x8);
    OLED_ShowString(115, 7, ">", FONT_SIZE_6x8);
    OLED_ShowString(55, 7, "OK", FONT_SIZE_6x8);
    
    /* 刷新屏幕 */
    OLED_Refresh();
    
    osDelay(200);  // 200ms刷新一次，避免闪烁
  }
}

/**
  * @brief  时钟任务函数
  * @param  argument: Not used
  * @retval None
  */
void ClockTaskFunc(void *argument)
{
  uint32_t tick_count = 0;
  uint8_t hours = 12, minutes = 30, seconds = 45;
  
  for(;;)
  {
    tick_count++;
    
    // 模拟时钟更新（实际应该用RTC）
    seconds++;
    if(seconds >= 60)
    {
      seconds = 0;
      minutes++;
      if(minutes >= 60)
      {
        minutes = 0;
        hours++;
        if(hours >= 24)
        {
          hours = 0;
        }
      }
    }
    
    // 每秒更新一次（实际项目应该用RTC中断）
    if(tick_count % 5 == 0)  // 5*200ms = 1秒
    {
      // 这里可以更新全局时钟变量
      // 实际项目中应该从RTC读取
    }
    
    osDelay(200);  // 200ms周期
  }
}

/**
  * @brief  系统监控任务函数
  * @param  argument: Not used
  * @retval None
  */
void SystemMonitorTaskFunc(void *argument)
{
  uint32_t last_wake_time = osKernelGetTickCount();
  uint32_t task_count = 0;
  
  for(;;)
  {
    task_count++;
    
    // 每10秒输出一次系统状态
    if(task_count % 50 == 0)  // 50*200ms = 10秒
    {
      // 获取任务堆栈使用情况（示例）
      // 实际可以使用uxTaskGetStackHighWaterMark()获取堆栈高水位
      
      // 简单的系统状态指示
      if(task_count % 100 == 0)  // 20秒闪烁一次
      {
        LED_SetMode(LED_MODE_BLINK_SLOW);
        osDelay(500);
        LED_SetMode(LED_MODE_OFF);
      }
    }
    
    // 固定周期运行
    osDelayUntil(last_wake_time + 200);
    last_wake_time = osKernelGetTickCount();
  }
}

/* USER CODE END Application */
