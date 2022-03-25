#include "MKL25Z4.h"                    // Device header
#include "ultrasonic.h"

//TPM2 C0 is for generating PWM for trigger pin
//TPM1 C0 is for counting time

void InitUltra() {
	//PIT initialization
	SIM_SCGC6 |= SIM_SCGC6_PIT_MASK;
	PIT->MCR &= ~PIT_MCR_MDIS_MASK;
	PIT->CHANNEL[0].TCTRL |= PIT_TCTRL_TIE_MASK;
	//GPIO initialization
	SIM_SCGC5 |= SIM_SCGC5_PORTD_MASK;
	//Configure to GPIO mode
	PORTD->PCR[TRIGGER_PIN] &= ~PORT_PCR_MUX_MASK;
	PORTD->PCR[TRIGGER_PIN] |= PORT_PCR_MUX(1);
	PORTD->PCR[ECHO_PIN] &= ~PORT_PCR_MUX_MASK;
	PORTD->PCR[ECHO_PIN] |= PORT_PCR_MUX(1);
	//Set PORT D PIN 1 as OUTPUT pin
	PTD->PDDR |= MASK(TRIGGER_PIN);
	//Set PORT D PIN 3 as INPUT pin
	PTD->PDDR &= ~MASK(ECHO_PIN);
}

static void delay(volatile uint32_t nof) {
  while(nof!=0) {
    __asm("NOP");
    nof--;
  }
}

void startUltra() {
	//Send a 10 us = 10^5 Hz logic 1 pulse to trigger pin to start the sensing process
	PIT->CHANNEL[0].LDVAL = 104; //for 10us pulse
	PIT->CHANNEL[1].LDVAL = 10485759;	//1s timer, which is definitely sufficient to detect distance
	PIT->CHANNEL[0].TCTRL &= ~PIT_TCTRL_TEN_MASK; 
	PIT->CHANNEL[0].TCTRL |= PIT_TCTRL_TEN_MASK;
	PIT->CHANNEL[1].TCTRL &= ~PIT_TCTRL_TEN_MASK;
	PIT->CHANNEL[1].TCTRL |= PIT_TCTRL_TEN_MASK;
	NVIC_SetPriority(PIT_IRQn, 1);
	NVIC_SetPriority(PORTD_IRQn, 2);
	NVIC_ClearPendingIRQ(PORTD_IRQn);
	NVIC_EnableIRQ(PIT_IRQn);
	NVIC_EnableIRQ(PORTD_IRQn);
}

volatile bool firstTime = true;
void PIT_IRQHandler() {
	//Clear Pending IRQ
	NVIC_ClearPendingIRQ(PIT_IRQn);
	//Configure output pin
	if (firstTime) {
		PORTD->PDOR |= MASK(TRIGGER_PIN);
		firstTime = false;
	}
	else {
		PORTD->PDOR &= ~MASK(TRIGGER_PIN);
		firstTime = true;
		NVIC_DisableIRQ(PIT_IRQn);
	}
	//Clear Flag
	PIT->CHANNEL[0].TFLG &= PIT_TFLG_TIF_MASK;
}

volatile int start_time = -2;
volatile int stop_time = -1;
volatile bool isRising = true;
void PORTD_IRQHandler() {
	// Clear Pending IRQ
	NVIC_ClearPendingIRQ(PORTD_IRQn);
	if (PORTD->ISFR & MASK(ECHO_PIN) & isRising) {
		start_time = PIT->CHANNEL[1].CVAL;
		isRising = false;
	}
	else if (PORTD->ISFR & MASK(ECHO_PIN) & !isRising) {
		stop_time = PIT->CHANNEL[1].CVAL;
		isRising = true;
	}
	//Clear INT Flag
	PORTD->ISFR |= 0xffffffff;
}
	
float getDistance() {
	startUltra();
	float distance = 0;
	float time = 0;
	while (start_time < 0 && stop_time < 0) {
		time = (float)(start_time - stop_time + 1) / 10485760; //convert to s
		distance = AIR_SPEED * time; //distance in cm
	}
	NVIC_DisableIRQ(PORTD_IRQn);
	return distance;
}
