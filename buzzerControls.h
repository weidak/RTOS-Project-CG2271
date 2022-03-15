#include "MKL25Z4.h"                    // Device header

#ifndef BUZZERCONTROLS_H_
#define BUZZERCONTROLS_H_

/*
Initialisation of PWM
*/
void initPWMBuzzer();

/*
TODO: Initialisation of GPIO for PORTD for buzzer and TPM Modes
*/
void InitGPIOBuzzer();
void playSong();

#endif