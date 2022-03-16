#include "MKL25Z4.h"                    // Device header

#ifndef LEDCONTROLS_H_
#define LEDCONTROLS_H_

/*
TODO: Initialisation of GPIO and PORTC to control flashing of LEDs
*/
void initGPIOLed();
void forwardState();
void reverseState();
void ledControl(uint8_t number);
void offLED();
void onLED(uint8_t number);
void onOffLED(uint8_t number);
void controlLED();
void onAllLED();

#endif