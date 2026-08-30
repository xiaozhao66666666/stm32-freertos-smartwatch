/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "event_groups.h"
#include "queue.h"
#include "semphr.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "beep.h"
#include "u8g2.h"
#include "Data.h"
/* USER CODE END Includes */

extern QueueHandle_t g_xQueueMenu;
extern TaskHandle_t xShowMenuTaskHandle;
extern u8g2_t u8g2;                  /* 全局实例（Data.c）：原函数内局部 u8g2_t 已删除 */
extern SemaphoreHandle_t g_ui_mtx;   /* freertos.c 单次创建 */

void ShowFlashLightTask(void *params)
{
	buzzer_init();
	/* 队列已收敛到 freertos.c 单次创建（迭代22） */
	if(NULL != g_xQueueMenu)HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);

	/* u8g2 Start */
	u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2,U8G2_R0, u8x8_byte_hw_i2c, u8g2_stm32_delay);
	u8g2_InitDisplay(&u8g2); // send init sequence to the display, display is in sleep mode after this,
	u8g2_SetPowerSave(&u8g2, 0); // wake up display
	u8g2_ClearDisplay(&u8g2);
//	u8g2_SetFont(&u8g2, u8g2_font_wqy16_t_chinese1);
//	u8g2_SetFont(&u8g2, u8g2_font_spleen32x64_mf);	
	u8g2_SetFont(&u8g2, u8g2_font_fur35_tf);
	
	//u8g2_ClearBuffer(&u8g2);
	u8g2_DrawXBMP(&u8g2, 48, 16, 30, 30, light);
	u8g2_SendBuffer(&u8g2);

	uint8_t light_flag = 0;
	struct Key_data	key_data;
	
	while(1)
	{
		/* 帧级加锁：ClearBuffer..SendBuffer 与其它任务共用同一 static 显存 */
		if(g_ui_mtx) xSemaphoreTake(g_ui_mtx, portMAX_DELAY);
		u8g2_ClearBuffer(&u8g2);
		/* 读按键中断队列 */
		xQueueReceive(g_xQueueMenu, &key_data, portMAX_DELAY);
		
		if(key_data.updata == 1)
		{
			buzzer_buzz(2500, 100);
			switch(light_flag)
			{
				case 0: u8g2_DrawBox(&u8g2, 0, 0, 128, 64);light_flag++;break;
				case 1: u8g2_DrawXBMP(&u8g2, 48, 16, 30, 30, light);light_flag--;break;
			}
		}		
		if(key_data.exdata == 1)
		{
			buzzer_buzz(2500, 100);
			vTaskResume(xShowMenuTaskHandle);
			vTaskSuspend(NULL);
		}		
		u8g2_SendBuffer(&u8g2);//我是SB
		if(g_ui_mtx) xSemaphoreGive(g_ui_mtx);
	}
}

