#include "MKL25Z4.h"                    // Device header

#ifndef ULTRASONIC_H_
#define ULTRASONIC_H_

#define TRIGGER_PIN 1	//PORTD PIN 1
#define ECHO_PIN 3	//PORTD PIN 3

#define MASK(x) (1UL << x)

#define AIR_SPEED 34000 //air speed in cm/s

void initUltra();

void startUltra();

float getDistance();

static void delay(volatile uint32_t nof);

#endif