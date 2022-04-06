#include "MKL25Z4.h"                    // Device header
#include "RTE_Components.h"             // Component selection
#include "cmsis_os2.h"                  // ::CMSIS:RTOS2

#ifndef BUZZERCONTROLS_H_
#define BUZZERCONTROLS_H_

#define MASK(x) (1<<(x))
#define BUZZER_PIN 0 //PORT B PIN 0 for TPM1_C0V
#define FREQ2MOD(x) 375000/x

extern osSemaphoreId_t buzzerSem;

/*
Initialisation of PWM
*/
void InitPWMBuzzer();

/*
TODO: Initialisation of GPIO for PORTD for buzzer and TPM Modes
*/
void InitGPIOBuzzer();
void playSong();
void playCompletedSong();
void playCoffin();

#endif