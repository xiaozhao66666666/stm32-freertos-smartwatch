#include "driver_dht11.h"

// 全局DHT11数据
DHT11_Data dht11_data = {0, 0, 0, 0};

// 微秒延迟函数
static void DHT11_Delay_us(uint16_t us)
{
    uint32_t ticks = us * (SystemCoreClock / 1000000) / 5;
    while(ticks--)
    {
        __NOP();
    }
}

/**
  * @brief  设置DHT11引脚为输出模式
  */
static void DHT11_Pin_Output(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    GPIO_InitStruct.Pin = DHT11_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStruct);
}

/**
  * @brief  设置DHT11引脚为输入模式
  */
static void DHT11_Pin_Input(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    GPIO_InitStruct.Pin = DHT11_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStruct);
}

/**
  * @brief  DHT11初始化
  */
void DHT11_Init(void)
{
    DHT11_Pin_Output();
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET);
}

/**
  * @brief  读取DHT11数据
  */
uint8_t DHT11_Read(DHT11_Data *data)
{
    uint8_t buffer[5] = {0};
    uint8_t checksum = 0;
    
    // 主机拉低至少18ms
    DHT11_Pin_Output();
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_RESET);
    osDelay(20);
    
    // 主机拉高20-40us
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET);
    DHT11_Delay_us(30);
    
    // 设置为输入模式等待DHT11响应
    DHT11_Pin_Input();
    
    // 等待DHT11拉低响应
    uint32_t timeout = 10000; // 超时计数
    while(HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_SET)
    {
        if(timeout-- == 0) return 0;
        DHT11_Delay_us(1);
    }
    
    // 等待DHT11拉高
    timeout = 10000;
    while(HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_RESET)
    {
        if(timeout-- == 0) return 0;
        DHT11_Delay_us(1);
    }
    
    // 等待DHT11开始发送数据
    timeout = 10000;
    while(HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_SET)
    {
        if(timeout-- == 0) return 0;
        DHT11_Delay_us(1);
    }
    
    // 读取40位数据
    for(uint8_t i = 0; i < 5; i++)
    {
        buffer[i] = 0;
        for(uint8_t j = 0; j < 8; j++)
        {
            // 等待低电平结束
            timeout = 10000;
            while(HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_RESET)
            {
                if(timeout-- == 0) return 0;
                DHT11_Delay_us(1);
            }
            
            // 测量高电平持续时间
            uint32_t high_time = 0;
            while(HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_SET)
            {
                high_time++;
                DHT11_Delay_us(1);
                if(high_time > 100) break; // 超时
            }
            
            // 判断位值（高电平持续时间>30us为1，否则为0）
            buffer[i] <<= 1;
            if(high_time > 30) // 大约30us阈值
            {
                buffer[i] |= 1;
            }
        }
    }
    
    // 验证校验和
    checksum = buffer[0] + buffer[1] + buffer[2] + buffer[3];
    if(checksum != buffer[4])
    {
        return 0;
    }
    
    // 解析数据
    data->humidity = (float)buffer[0] + (float)buffer[1] / 10.0;
    data->temperature = (float)buffer[2] + (float)buffer[3] / 10.0;
    data->checksum = buffer[4];
    data->valid = 1;
    
    return 1;
}

/**
  * @brief  DHT11任务函数
  */
void DHT11_Task(void *argument)
{
    TickType_t last_wake_time = osKernelGetTickCount();
    const TickType_t period = 2000; // 2秒读取一次
    
    DHT11_Init();
    
    for(;;)
    {
        if(DHT11_Read(&dht11_data))
        {
            // 读取成功，数据已存储在dht11_data中
        }
        else
        {
            // 读取失败
            dht11_data.valid = 0;
        }
        
        osDelayUntil(last_wake_time + period);
        last_wake_time = osKernelGetTickCount();
    }
}