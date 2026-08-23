# 第三章：FreeRTOS多任务设计 — 核心架构

## 3.1 为什么需要RTOS？

### 裸机 vs RTOS 对比

| 场景 | 裸机 (Super Loop) | FreeRTOS |
|------|-------------------|----------|
| 按键扫描 | 轮询，可能丢键 | 中断+队列，不丢 |
| OLED刷新 | 阻塞delay() | 独立任务，非阻塞 |
| 多应用切换 | if-else状态机，耦合严重 | 任务挂起/恢复，解耦 |
| 定时更新 | 软件延时，不准 | 硬件定时器，精确 |
| 代码维护 | 一个loop()越来越大 | 每个功能独立.c文件 |

### 本项目任务拓扑

```
优先级: 高 ←────────────────────────────→ 低

[按键中断] → 队列 → [ShowMenuTask] ─┬─ vTaskResume → [ShowCalendarTask]
  (ISR)      (Queue)   (Normal)      ├─ vTaskResume → [ShowClockTask]
                                     ├─ vTaskResume → [ShowDHT11Task]
                                     ├─ vTaskResume → [ShowFlashLightTask]
                                     ├─ vTaskResume → [ShowSettingTask]
                                     └─ vTaskResume → [ShowTimeTask]

[软件定时器1] ──1秒──▶ TimerCallBackFun (更新RTC时间)
[软件定时器2] ──100ms──▶ ClockTimerCallBackFun (刷新显示)
```

## 3.2 FreeRTOS核心概念（面试必问）

### 1. 任务 (Task)
```c
// 创建任务
xTaskCreate(
    ShowMenuTask,       // 任务函数
    "ShowMenuTask",     // 任务名（调试用）
    256,                // 栈大小（字，不是字节！）
    NULL,               // 参数
    osPriorityNormal,   // 优先级
    &xShowMenuTaskHandle // 任务句柄（用于挂起/恢复）
);
```

### 2. 消息队列 (Queue)
```c
// 按键中断 → 队列 → 菜单任务
// 创建队列（4个槽位，每槽4字节）
g_xQueueMenu = xQueueCreate(4, 4);

// ISR中发送（不可阻塞）
xQueueSendToBackFromISR(g_xQueueMenu, &key_data, NULL);

// 任务中接收（阻塞等待）
xQueueReceive(g_xQueueMenu, &key_data, portMAX_DELAY);
```

**为什么用队列而不是全局变量？**
- 全局变量在ISR和任务间共享需要volatile+临界区保护
- 队列天然线程安全，FreeRTOS内部处理了同步
- 队列可以缓存多次按键（如果任务来不及处理）

### 3. 任务挂起/恢复（本项目核心设计模式）
```c
// 菜单任务选择应用后：
vTaskResume(xShowCalendarTaskHandle);  // 唤醒日历任务
vTaskSuspend(NULL);                    // 挂起自己（菜单）

// 日历任务退出时：
vTaskResume(xShowMenuTaskHandle);     // 唤醒菜单
vTaskSuspend(NULL);                    // 挂起自己
```

**这就是整个手表的多应用切换机制！** 任何时候只有一个应用任务+菜单任务在运行，其余全部挂起，节省CPU和内存。

### 4. 软件定时器 (Software Timer)
```c
// 1秒定时器：更新系统时间
g_Timer = xTimerCreate(
    "Timer1", 1000,         // 周期1000ms
    pdTRUE,                 // 自动重载
    NULL,                   // 参数
    TimerCallBackFun        // 回调函数
);
xTimerStart(g_Timer, 0);

// 回调函数在定时器服务任务中执行，不能阻塞！
void TimerCallBackFun(TimerHandle_t xTimer) {
    // 更新秒数、发送刷新信号...
}
```

## 3.3 按键驱动架构（精妙设计）

```
硬件: GPIO PC11(右) PC10(左) PB1(确定) PB0(返回)
       ↓ 外部中断 (下降沿触发)
ISR: HAL_GPIO_EXTI_Callback()
       ↓ 软件消抖 (for循环5000次)
       ↓ 填充 Key_data 结构体
       ↓ xQueueSendToBackFromISR()
       ↓
任务: ShowMenuTask
       ↓ xQueueReceive() (阻塞等待)
       ↓ 解析 rdata/ldata/updata/exdata
       ↓ 更新UI / 切换应用
```

### 按键数据结构
```c
typedef struct Key_data {
    uint8_t rdata;   // 右键 (→)
    uint8_t ldata;   // 左键 (←)
    uint8_t updata;  // 返回键 (↑)
    uint8_t exdata;  // 确定键 (✓)
} Key_data;
```

### 按键消抖
```c
// 软件延时消抖（简单但有效）
if(GPIO_Pin == GPIO_PIN_11) {
    for(int i = 0; i < 5000; i++) {}  // 约10ms延时
    // ... 处理按键
}
```

## 3.4 内存管理

### STM32F103C8 资源限制
| 资源 | 大小 | 本项目使用 |
|------|------|-----------|
| Flash | 64KB | ~40KB |
| SRAM | 20KB | ~16KB |
| 堆栈 | 由FreeRTOS管理 | 每任务128~256字 |

### 栈溢出保护
```c
// FreeRTOSConfig.h 中启用
#define configCHECK_FOR_STACK_OVERFLOW  2

// 任务创建时合理分配栈大小
xTaskCreate(ShowMenuTask, "...", 256, ...);  // 菜单：256字=1KB
xTaskCreate(ShowDHT11Task, "...", 128, ...); // 简单任务：128字=512B
```

## 3.5 任务状态转换图

```
                    ┌──────────┐
                    │  Running │  (当前CPU执行的任务)
                    └──────────┘
                         │
            ┌────────────┼────────────┐
            ▼            ▼            ▼
      ┌──────────┐ ┌──────────┐ ┌──────────┐
      │  Ready   │ │ Blocked  │ │Suspended │
      │ (就绪)   │ │ (阻塞)   │ │ (挂起)   │
      └──────────┘ └──────────┘ └──────────┘
      等待调度器   等待队列/    vTaskSuspend()
      分配CPU      信号量/     后直到
                   延时结束    vTaskResume()
```

本项目中：
- **菜单任务**：大多数时间 Blocked（等队列数据）
- **应用任务**：大多数时间 Suspended（等用户选择）
- **定时器回调**：在Timer Service Task中短暂执行
- **空闲任务**：所有任务阻塞/挂起时运行，进入低功耗
