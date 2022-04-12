#include "MKL25Z4.h"                    // Device header
#include "cmsis_os2.h"                  // ::CMSIS:RTOS2
#include "RTE_Components.h"             // Component selection

#ifndef MOTORCONTROLS_H_
#define MOTORCONTROLS_H_

/***********
Available GPIO for TPM0
CH0 -> PORTD 0
CH1 -> PORTD 1
CH2 -> PORTD 2
CH3 -> PORTD 3
***********/

#define FORW_RIGHT_MOTORS 0	//PORTD PIN 0
#define REV_LEFT_MOTORS 1		//PORTD PIN 1
#define FORW_LEFT_MOTORS 2	//PORTD PIN 2
#define REV_RIGHT_MOTORS 3	//PORTD PIN 3

#define FORW_RIGHT_WHEELS TPM0_C0V
#define REV_LEFT_WHEELS TPM0_C1V
#define FORW_LEFT_WHEELS TPM0_C2V
#define REV_RIGHT_WHEELS TPM0_C3V

#define FULL_SPEED 7501
#define MEDIUM_SPEED 5000
#define HALF_SPEED 4000
#define SLOW_SPEED 3000

#define MASK(x) (1UL << x)

/*
Initialisation of PWM
*/
void InitPWMMotors();

/*
Initialisation of GPIO 
*/
void InitGPIOMotors();

/*
Movement Controls
*/
void forwards(uint32_t speed);
void reverse(uint32_t speed);
void stop_moving();
void move(uint32_t cmd, uint32_t speed);

void left(uint32_t speed);

//Stationary turn:
void left45(uint32_t speed);

void right(uint32_t speed);

//Stationary turn:
void right90(uint32_t speed);

void left_stationary(uint32_t speed);
void right_stationary(uint32_t speed);


#endif