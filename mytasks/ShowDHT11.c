/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "event_groups.h"
#include "queue.h"
#include "semphr.h"
#include <stdio.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "beep.h"
#include "driver_dht11.h"
#include "u8g2.h"
#include "Data.h"
#include "ShowCalendar.h"

/* USER CODE END Includes */

extern TaskHandle_t xShowMenuTaskHandle;
extern QueueHandle_t g_xQueueMenu;
extern u8g2_t u8g2;                  /* 全局实例（Data.c）：原函数内局部 u8g2_t 已删除 */
extern SemaphoreHandle_t g_ui_mtx;   /* freertos.c 单次创建 */

void ShowDHT11Task(void *params)
{
	DHT11_Init();
	buzzer_init();
	/* 队列已收敛到 freertos.c 单次创建（迭代22） */
	if(NULL != g_xQueueMenu)HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);

	u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2,U8G2_R0, u8x8_byte_hw_i2c, u8g2_stm32_delay);
	u8g2_InitDisplay(&u8g2); // send init sequence to the display, display is in sleep mode after this,
	u8g2_SetPowerSave(&u8g2, 0); // wake up display
	u8g2_ClearDisplay(&u8g2);
	u8g2_SetFont(&u8g2, u8g2_font_wqy16_t_chinese1);

	u8g2_SendBuffer(&u8g2);
	
	struct Key_data	key_data;
	int hum, temp;
    int hum1, hum2, hum3, temp1, temp2 ,temp3;

    int max = 20;   /* 初始化为最低档：原为未初始化读取（UB），档位判断语义不变 */
	int g_max[] = {20, 30, 40, 50, 60, 70};
	
	while(1)
	{
		/* 帧级加锁：ClearBuffer..SendBuffer 与其它任务共用同一 static 显存 */
		if(g_ui_mtx) xSemaphoreTake(g_ui_mtx, portMAX_DELAY);
		u8g2_ClearBuffer(&u8g2);
		if (DHT11_Read(&hum, &temp) !=0 ){
			//printf("\n\rdht11 read err!\n\r");
			DHT11_Init();
		}
		else{
			temp1 = temp%10;	//个位（原 %20 在 30℃ 时算出 10，数码显示错乱）
			temp3 = temp/10;
			temp2 = temp3%10;   //十位
						
			for(int i=0; i<5; i++)
			{
				max = hum>g_max[i]?g_max[i]:max;
			}			
			hum1 = hum%max;		//low bit	
			hum3 = hum/10;
			hum2 = hum3%max;    //high bit
			
			u8g2_DrawXBMP(&u8g2, 10, 20, 20, 40, BigNum[temp2]);
			u8g2_DrawXBMP(&u8g2, 35, 20, 20, 40, BigNum[temp1]);
			
			u8g2_DrawXBMP(&u8g2, 75, 20, 20, 40, BigNum[hum2]);
			u8g2_DrawXBMP(&u8g2, 100, 20, 20, 40, BigNum[hum1]);
		}
		u8g2_DrawStr(&u8g2, 15, 15, "temp");
		u8g2_DrawStr(&u8g2, 85, 15, "Hum");

		u8g2_SendBuffer(&u8g2);
		if(g_ui_mtx) xSemaphoreGive(g_ui_mtx);

		/* 读按键中断队列 */
		xQueueReceive(g_xQueueMenu, &key_data, 0);
		
		if(key_data.exdata == 1)
		{
			buzzer_buzz(2500, 100);
			vTaskResume(xShowMenuTaskHandle);
			vTaskSuspend(NULL);
			key_data.exdata = 0;
		}
	}
}
