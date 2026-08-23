#ifndef __BUZZER_H
#define __BUZZER_H

#include "main.h"
#include "tim.h"

// 蜂鸣器音调定义
typedef enum {
    BUZZER_OFF = 0,
    BUZZER_C4 = 262,  // Do
    BUZZER_D4 = 294,  // Re
    BUZZER_E4 = 330,  // Mi
    BUZZER_F4 = 349,  // Fa
    BUZZER_G4 = 392,  // Sol
    BUZZER_A4 = 440,  // La
    BUZZER_B4 = 494,  // Si
    BUZZER_C5 = 523,  // Do
    BUZZER_D5 = 587,  // Re
    BUZZER_E5 = 659,  // Mi
    BUZZER_F5 = 698,  // Fa
    BUZZER_G5 = 784,  // Sol
    BUZZER_A5 = 880,  // La
    BUZZER_B5 = 988,  // Si
} BuzzerTone;

// 蜂鸣器模式
typedef enum {
    BUZZER_MODE_OFF = 0,
    BUZZER_MODE_ON,
    BUZZER_MODE_BEEP,
    BUZZER_MODE_MELODY
} BuzzerMode;

// 函数声明
void BUZZER_Init(void);
void BUZZER_SetTone(BuzzerTone tone);
void BUZZER_On(void);
void BUZZER_Off(void);
void BUZZER_Beep(uint16_t duration_ms);
void BUZZER_SetMode(BuzzerMode mode);
void BUZZER_Update(void);
void BUZZER_PlayMelody(const BuzzerTone *melody, const uint16_t *durations, uint16_t length);

#endif