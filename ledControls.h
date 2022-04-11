#include "MKL25Z4.h"  // Device header
#include "RTE_Components.h"             // Component selection
#include "cmsis_os2.h"                  // ::CMSIS:RTOS2

#ifndef LEDCONTROLS_H_
#define LEDCONTROLS_H_

//Initialisation of GPIO and PORTC to control flashing of LEDs
void InitGPIOLed();

//Helper Functions
void offLED(uint8_t number);
void onLED(uint8_t number);
void onOffLED(uint8_t number);
void runningFrontLED();
void onAllLED();
void rearLed500();
void rearLed250();

#endif