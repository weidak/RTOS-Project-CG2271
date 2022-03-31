#include "MKL25Z4.h"                    // Device header

#ifndef ULTRASONIC_H_
#define ULTRASONIC_H_

#define TRIGGER_PIN 30	//PORTE PIN 30
#define ECHO_PIN 31	//PORTE PIN 31

#define MASK(x) (1UL << x)

#define AIR_SPEED 34000 //air speed in cm/s

void InitUltra();

float getDistance();

static void delay(volatile uint32_t nof);

#endif