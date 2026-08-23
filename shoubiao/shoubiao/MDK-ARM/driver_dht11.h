#ifndef __DHT11_H
#define __DHT11_H

#include "main.h"
#include "cmsis_os.h"

// DHT11引脚定义
#define DHT11_PIN  GPIO_PIN_1
#define DHT11_PORT GPIOA

// DHT11数据结构
typedef struct {
    float temperature;
    float humidity;
    uint8_t checksum;
    uint8_t valid;
} DHT11_Data;

// 函数声明
void DHT11_Init(void);
uint8_t DHT11_Read(DHT11_Data *data);
void DHT11_Task(void *argument);

// 全局DHT11数据
extern DHT11_Data dht11_data;

#endif