#include "MKL25Z4.h"  // Device header
#include "RTE_Components.h"             // Component selection
#include "cmsis_os2.h"                  // ::CMSIS:RTOS2
#include "motorControls.h"

#ifndef ULTRASONIC_H_
#define ULTRASONIC_H_

#define TRIGGER_PIN 30	//PORTE PIN 30
#define ECHO_PIN 31	//PORTE PIN 31

#define MASK(x) (1UL << x)

#define AIR_SPEED 34000 //air speed in cm/s

//Speeds of turns and forwards
#define SD_SPEED HALF_SPEED

//Current distance threshold is at 20cm
#define DISTANCE_THRESHOLD 20 

//Delays subjected to changes
#define DELAY_LEFT_TURN 2000 //Enough delay for bot to complete a 45 degree left turn
#define DELAY_RIGHT_TURN 2000 //Enough delay for bot to complete a 90 degree right turn
#define DELAY_STRAIGHT 2000 //move forwards for half a second?
#define DELAY_STOP 2000 //stop and chill for a while before turning right

void InitUltra();

float getDistance();

static void delay(volatile uint32_t nof);

#endif