# RTOS智能手表项目 — 完整复刻指南

> STM32F103 + FreeRTOS + u8g2 OLED · 多任务智能手表系统

---

## 📖 项目概述

### 这是什么项目？
基于STM32F103C8（ARM Cortex-M3，72MHz）和FreeRTOS实时操作系统的智能手表。通过u8g2图形库驱动128×64 OLED屏幕，实现了**多任务菜单系统**，包含5个应用：日历、手电筒、温湿度(DHT11)、秒表时钟、设置菜单。

### 为什么叫"手表"（shoubiao）？
整个系统设计为可穿戴形态，通过4个按键（上下左右）进行菜单导航，OLED屏显示，蜂鸣器反馈，电池供电。

### 硬件平台
| 组件 | 型号 | 协议 |
|------|------|------|
| 主控 | STM32F103C8T6 (Cortex-M3) | - |
| 显示屏 | SSD1306 128×64 OLED | I2C |
| 温湿度 | DHT11 | One-Wire |
| 按键 | 4× 轻触开关 | GPIO中断 |
| 蜂鸣器 | 无源蜂鸣器 | PWM |
| 时钟源 | 8MHz HSE → PLL 72MHz | - |

### 核心技术栈
| 层级 | 技术 |
|------|------|
| MCU | STM32F103C8T6 (72MHz, 20KB SRAM, 64KB Flash) |
| RTOS | FreeRTOS v10 (CMSIS-OS v2 封装) |
| HAL库 | STM32CubeMX HAL |
| 图形库 | u8g2 (SSD1306驱动) |
| IDE | Keil MDK-ARM + STM32CubeMX |
| 调试 | J-Link / ST-Link |

---

## 📁 目录结构

```
复刻/
├── README.md                        ← 总索引
├── 01_项目概述与架构.md
├── 02_环境搭建.md
├── 03_FreeRTOS多任务设计.md          ← ★ 核心
├── 04_u8g2图形与菜单系统.md
├── 05_外设驱动详解.md
├── 06_复刻步骤.md
└── code/
    ├── Core/                        ← HAL初始化代码
    ├── mytasks/                     ← ★ FreeRTOS任务模块
    │   ├── Data.h / Data.c          ← 数据结构&位图
    │   ├── ShowMenu.c               ← 主菜单任务
    │   ├── ShowTimeTask.c           ← 时间/状态栏
    │   ├── ShowCalendar.c           ← 日历应用
    │   ├── ShowClock.c              ← 秒表时钟
    │   ├── ShowDHT11.c              ← 温湿度显示
    │   ├── ShowFlashLight.c         ← 手电筒
    │   ├── ShowSetting.c            ← 设置菜单
    │   └── ShowWoodenFish.c         ← 木鱼游戏
    ├── Drivers/                     ← HAL库
    ├── u8g2/                        ← u8g2图形库
    └── hal_freertos_u8g2.ioc        ← CubeMX工程文件
```

---

## 🎯 核心创新（面试重点）

**在20KB SRAM限制下，用FreeRTOS管理10+个并发任务，通过消息队列+任务挂起/恢复实现了完整的菜单导航和5应用调度系统。**

```
按键中断 ──Queue──▶ ShowMenuTask ──vTaskResume──▶ 5个应用任务
                         │
                    任务调度器 (FreeRTOS Scheduler)
                         │
          ┌──────────────┼──────────────┐
          ▼              ▼              ▼
     ShowCalendar   ShowDHT11    ShowClock ...
     (挂起等待)     (挂起等待)   (挂起等待)
```

关键技术：
1. **FreeRTOS多任务**：10+任务并行，抢占式调度
2. **消息队列**：按键中断→队列→菜单任务，解耦ISR和业务逻辑
3. **任务挂起/恢复**：`vTaskSuspend(NULL)` + `vTaskResume()` 实现应用切换
4. **软件定时器**：1秒定时更新时钟，100ms刷新显示
5. **u8g2图形**：页缓冲模式，I2C硬件驱动，中文字体支持

---

## ⚡ 快速开始

```bash
# 1. 安装 STM32CubeMX + Keil MDK
# 2. 打开 hal_freertos_u8g2.ioc → 生成代码
# 3. 添加 mytasks/ 和 u8g2/ 到工程
# 4. 编译 → 下载 (ST-Link)
# 5. OLED显示菜单界面
```
