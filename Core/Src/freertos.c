/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "event_groups.h"
#include "queue.h"
#include "semphr.h"
#include "u8g2.h"
#include "beep.h"
#include "Data.h"
#include "ShowTimeTask.h"
#include "ShowMenu.h"
//#include "ShowCalendar.h"
//#include "ShowClock.h"
//#include "ShowFlashLight.h"
//#include "ShowSetting.h"
//#include "ShowWoodenFish.h"
//#include "ShowHRSPO2.h"
//#include "RootTask.h"
/*dongshan_driver*/
//#include "driver_dht11.h"
//#include "driver_passive_buzzer.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define Task_default_size 256   /* 128→256：512B 栈放不下 u8g2 局部调用链 */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
/* some task handle */
TimerHandle_t g_Timer;
TimerHandle_t g_Clock_Timer;

TaskHandle_t xRootTaskHandle = NULL;

TaskHandle_t xShowTimeTaskHandle = NULL;
TaskHandle_t xShowMenuTaskHandle = NULL;
TaskHandle_t xShowCalendarTaskHandle = NULL;
TaskHandle_t xShowClockTaskHandle = NULL;
TaskHandle_t xShowFlashLightTaskHandle = NULL;
TaskHandle_t xShowSettingTaskHandle = NULL;
TaskHandle_t xShowWoodenFishTaskHandle = NULL;
TaskHandle_t xShowDHT11TaskHandle = NULL;
TaskHandle_t xShowHRSPO2TaskHandle = NULL;

QueueHandle_t g_xQueueMenu;
SemaphoreHandle_t g_ui_mtx;   /* u8g2 全局帧缓冲互斥（显示硬件+static 显存单份） */
uint16_t key1_filter = 0;
uint16_t key2_filter = 0;
uint16_t key3_filter = 0;
uint16_t key4_filter = 0;

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

extern void ClockTimerCallBackFun(void);

extern void ShowDHT11Task(void *params);
extern void ShowCalendarTask(void *params);
extern void ShowFlashLightTask(void *params);
extern void ShowWoodenFishTask(void *params);
extern void ShowClockTimeTask(void *params);
extern void ShowSetting_Task(void *params);

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
	buzzer_init();
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
	/* time and clock's Timer */
	g_Timer = xTimerCreate("Timer1",
				1000,
				pdTRUE,
				NULL,
				(TimerCallbackFunction_t)TimerCallBackFun);
	
	g_Clock_Timer = xTimerCreate("Timer2",
				100,
				pdTRUE,
				NULL,
				(TimerCallbackFunction_t)ClockTimerCallBackFun);
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);
//  xTaskCreate(RootTask, "RootTask", 128, NULL, osPriorityNormal, &xRootTaskHandle);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  
  /* create some tasks */
	/* 单次初始化：原来 ShowTimeTask(len1)/ShowMenuTask(len4) 各自重建队列，
	 * 句柄互相覆盖、旧队列泄漏、ISR 事件丢失——现统一在此创建一次 */
	g_xQueueMenu = xQueueCreate(4, sizeof(Key_data));
	g_ui_mtx = xSemaphoreCreateMutex();

	xTaskCreate(ShowTimeTask, "ShowTimeTask", 512, NULL, osPriorityNormal, &xShowTimeTaskHandle);   /* 128→512：栈上有 u8g2_t */
	xTaskCreate(ShowMenuTask, "ShowMenuTask", 256, NULL, osPriorityNormal, &xShowMenuTaskHandle);

/******** 5 apps ********/
	/*1*/
  	xTaskCreate(ShowCalendarTask, "ShowCalendarTask", 256, NULL, osPriorityNormal, &xShowCalendarTaskHandle);
	/*2*/
  	xTaskCreate(ShowFlashLightTask, "ShowFlashLightTask", Task_default_size, NULL, osPriorityNormal, &xShowFlashLightTaskHandle);
    /*3*/
  	xTaskCreate(ShowDHT11Task, "ShowDHT11Task", 512, NULL, osPriorityNormal, &xShowDHT11TaskHandle);  /* Task_default_size→512 */
	//xTaskCreate(ShowWoodenFishTask, "ShowWoodenFishTask", Task_default_size, NULL, osPriorityNormal, &xShowWoodenFishTaskHandle);
    /*4*/
  	xTaskCreate(ShowClockTimeTask, "ShowClockTimeTask", Task_default_size, NULL, osPriorityNormal, &xShowClockTaskHandle);
	/*5*/
  	xTaskCreate(ShowSetting_Task, "ShowSetting_Task", 256, NULL, osPriorityNormal, &xShowSettingTaskHandle);

//	PassiveBuzzer_Test();
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
	
	/* launch time Timer */
	if(g_Timer != NULL)
	{
		xTimerStart(g_Timer, 0);
	}
	for(;;)
	{
		osDelay(1);
	}
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	/* key interrupt : send data to queue
	 * 修复：原实现 ISR 内 for(5000) 忙等"消抖"且读非 volatile 任务变量——
	 * 现改为时间戳消抖（非阻塞）+ volatile 读取；消抖窗口 200ms。 */
	extern volatile uint32_t end_flag;      /* 定义在 ShowMenu.c（已加 volatile） */
	extern volatile uint32_t seclect_end;
	static uint32_t last_ms[4] = {0, 0, 0, 0};
	uint32_t now = HAL_GetTick();
	int idx = (GPIO_Pin == GPIO_PIN_11) ? 0 : (GPIO_Pin == GPIO_PIN_10) ? 1
	        : (GPIO_Pin == GPIO_PIN_1)  ? 2 : 3;
	Key_data key_data;

	if (now - last_ms[idx] < 200) return;   /* 消抖窗口 */
	last_ms[idx] = now;

    if(GPIO_Pin == GPIO_PIN_11)
	{
		if(end_flag == 1&&seclect_end == 0)
		{
			key_data.rdata = 1;
			xQueueSendToBackFromISR(g_xQueueMenu, &key_data, NULL);
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
		}
	}
	if(GPIO_Pin == GPIO_PIN_10)
	{
		if(end_flag == 1&&seclect_end == 0)
		{
		 	key_data.ldata = 1;
			xQueueSendToBackFromISR(g_xQueueMenu, &key_data, NULL);
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
		}
	}
	if(GPIO_Pin == GPIO_PIN_1)
	{
		if(end_flag == 1&&seclect_end == 0)
		{
			key_data.updata = 1;
			xQueueSendToBackFromISR(g_xQueueMenu, &key_data, NULL);
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
		}
	}
	if(GPIO_Pin == GPIO_PIN_0)
	{
		if(end_flag == 1&&seclect_end == 0)
		{
			key_data.exdata = 1;
			xQueueSendToBackFromISR(g_xQueueMenu, &key_data, NULL);
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
		}
	}
}
/* USER CODE END Application */

