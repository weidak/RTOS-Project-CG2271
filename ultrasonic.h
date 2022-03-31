#include "MKL25Z4.h"                    // Device header

#ifndef ULTRASONIC_H_
#define ULTRASONIC_H_

#define TRIGGER_PIN 2	//PORTD PIN 2
#define ECHO_PIN_RISING 3	//PORTD PIN 3
#define ECHO_PIN_FALLING 16 //PORTA PIN 16

#define MASK(x) (1UL << x)

#define AIR_SPEED 34000 //air speed in cm/s

void InitUltra();

void startUltra();

float getDistance();

static void delay(volatile uint32_t nof);

#endif