#include "MKL25Z4.h"                    // Device header

#ifndef BUZZERCONTROLS_H_
#define BUZZERCONTROLS_H_

/*
TODO: Initialisation of GPIO and PORTx to control flashing of LEDs
*/
void initGPIOLed();
void forwardState();
void reverseState();
void ledControl();

#endif