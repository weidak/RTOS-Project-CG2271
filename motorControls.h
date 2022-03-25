#include "MKL25Z4.h"                    // Device header

#ifndef MOTORCONTROLS_H_
#define MOTORCONTROLS_H_

#define FORW_LEFT_MOTORS 2	//PORTB PIN 2
#define REV_LEFT_MOTORS 3	//PORTB PIN 2
#define FORW_RIGHT_MOTORS 0	//PORTB PIN 0
#define REV_RIGHT_MOTORS 1	//PORTB PIN 1

//LEFT will be controlled by TPM2, RIGHT will be controlled by TPM1, with FWD = C0V and REV = C1V
#define FORW_LEFT_WHEELS TPM2_C0V	
#define REV_LEFT_WHEELS TPM2_C1V
#define FORW_RIGHT_WHEELS TPM1_C0V
#define REV_RIGHT_WHEELS TPM1_C1V

#define FULL_SPEED 7501
#define HALF_SPEED 3500
#define SLOW_SPEED 1000

#define MASK(x) (1UL << x)

/*
Initialisation of PWM
*/
void InitPWMMotors();

/*
Initialisation of GPIO for PORTB GPIO and TPM Modes
*/
void InitGPIOMotors();

/*
Movement Controls
*/
void forwards(uint32_t speed);
void reverse(uint32_t speed);
void stop_moving();

void left(uint32_t speed);

//Stationary turn:
void left90();

void right(uint32_t speed);

//Stationary turn:x
void right90();

static void delay(volatile uint32_t nof);

#endif