#include "MKL25Z4.h"                    // Device header

#ifndef LEDCONTROLS_H_
#define LEDCONTROLS_H_


//Initialisation of GPIO and PORTC to control flashing of LEDs
void initGPIOLed();

//TODO: States
void forwardState();
void reverseState();

//Helper Functions
void offLED(uint8_t number);
void onLED(uint8_t number);
void onOffLED(uint8_t number);
void runningFrontLED(uint32_t* cmd);
void onAllLED();
void rearLed500();
void rearLed250();

#endif