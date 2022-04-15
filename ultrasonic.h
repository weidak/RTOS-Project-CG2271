#include "MKL25Z4.h"  // Device header
#include "RTE_Components.h"             // Component selection
#include "cmsis_os2.h"                  // ::CMSIS:RTOS2
#include "motorControls.h"

#ifndef ULTRASONIC_H_
#define ULTRASONIC_H_

#define TRIGGER_PIN 31	//PORTE PIN 31
#define ECHO_PIN 2	//PORTB PIN 2

#define MASK(x) (1UL << x)

#define AIR_SPEED 34000 //air speed in cm/s

//Speeds of turns and forwards
#define SD_SPEED MEDIUM_SPEED
#define FIRST_FORWARD_SPEED 4000
#define RETURN_FORWARD_SPEED 7501


//Current distance threshold is at 20cm
#define DISTANCE_THRESHOLD 45

//Delays subjected to changes, currently calibrated at SD_SPEED = HALF_SPEED (4000)
#define DELAY_LEFT_TURN 180 //Enough delay for bot to complete a 45 degree left turn
#define DELAY_RIGHT_TURN 340 //Enough delay for bot to complete a 90 degree right turn
#define DELAY_STRAIGHT 360 //move forwards for half a second?
#define DELAY_STOP 100 //stop and chill for a while before turning right
#define DELAY_REV 200

void InitUltra();

float getDistance();

static void delay(volatile uint32_t nof);

#endif