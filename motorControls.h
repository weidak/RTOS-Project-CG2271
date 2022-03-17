#include "MKL25Z4.h"                    // Device header

#ifndef MOTORCONTROLS_H_
#define MOTORCONTROLS_H_

#define FORW_UPPER_LEFT_MOTOR 0		//PORTB PIN 0
#define FORW_BOTTOM_LEFT_MOTOR 1	  //PORTB PIN 1
#define FORW_UPPER_RIGHT_MOTOR 2		//PORTB PIN 2
#define FORW_BOTTOM_RIGHT_MOTOR 3  //PORTB PIN 3

#define REV_UPPER_LEFT_MOTOR 12			//PORTA PIN 12
#define REV_BOTTOM_LEFT_MOTOR 13	  //PORTA PIN 13
#define REV_UPPER_RIGHT_MOTOR 1		//PORTA PIN 1
#define REV_BOTTOM_RIGHT_MOTOR 2  //PORTA PIN 2

#define UPPER_LEFT_WHEEL TPM1_C0V 
#define BOTTOM_LEFT_WHEEL TPM1_C1V
#define UPPER_RIGHT_WHEEL TPM2_C0V
#define BOTTOM_RIGHT_WHEEL TPM2_C1V

#define FULL_SPEED 7501
#define HALF_SPEED 3500

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
void stop();

void left(uint32_t speed);

//Stationary turn:
void left90();

void right(uint32_t speed);

//Stationary turn:
void right90();

static void delay(volatile uint32_t nof);

#endif