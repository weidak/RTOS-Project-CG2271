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
	SIM_SCGC5 |= SIM_SCGC5_PORTD_MASK | SIM_SCGC5_PORTA_MASK;
	//Configure to GPIO mode
	PORTD->PCR[TRIGGER_PIN] &= ~PORT_PCR_MUX_MASK;
	PORTD->PCR[TRIGGER_PIN] |= PORT_PCR_MUX(1);
	PORTD->PCR[ECHO_PIN_RISING] &= ~PORT_PCR_MUX_MASK;
	PORTD->PCR[ECHO_PIN_RISING] |= (PORT_PCR_MUX(1) | PORT_PCR_IRQC(0x9) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK); //rising edge detection on PORTD ISR 
	PORTA->PCR[ECHO_PIN_FALLING] &= ~PORT_PCR_MUX_MASK;
	PORTA->PCR[ECHO_PIN_FALLING] |= (PORT_PCR_MUX(1) | PORT_PCR_IRQC(0xa) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK); //falling edge detection on PORTA ISR
	//Set PORT D PIN 1 as OUTPUT pin
	PTD->PDDR |= MASK(TRIGGER_PIN);
	//Set PORT D PIN 3 as INPUT pin
	PTD->PDDR &= ~MASK(ECHO_PIN_RISING);
	//Set PORT A PIN 16 as INPUT pin
	PTA->PDDR &= ~MASK(ECHO_PIN_FALLING);
	//Set and enable interrupts
	NVIC_SetPriority(PIT_IRQn, 1);
	NVIC_SetPriority(PORTD_IRQn, 1);
	NVIC_SetPriority(PORTA_IRQn, 1);
	NVIC_ClearPendingIRQ(PIT_IRQn);
	NVIC_ClearPendingIRQ(PORTD_IRQn);
	NVIC_ClearPendingIRQ(PORTA_IRQn);
	NVIC_EnableIRQ(PIT_IRQn);
	NVIC_EnableIRQ(PORTD_IRQn);
	NVIC_EnableIRQ(PORTA_IRQn);
}

static void delay(volatile uint32_t nof) {
  while(nof!=0) {
    __asm("NOP");
    nof--;
  }
}

volatile int firstTime = 0;
volatile uint32_t start_time = 0;
volatile uint32_t stop_time = 0;

void startUltra() {
	//Send a 10 us = 10^5 Hz logic 1 pulse to trigger pin to start the sensing process
	start_time = 0;
	stop_time = 0;
	firstTime = 0;
	PIT->CHANNEL[0].LDVAL = 104;//for 10us pulse
	PIT->CHANNEL[1].LDVAL = 10485759;	//1s timer, which is definitely sufficient to detect distance
	PIT->CHANNEL[0].TCTRL &= ~PIT_TCTRL_TEN_MASK; 
	PIT->CHANNEL[0].TCTRL |= PIT_TCTRL_TEN_MASK;
	PIT->CHANNEL[1].TCTRL &= ~PIT_TCTRL_TEN_MASK;
	PIT->CHANNEL[1].TCTRL |= PIT_TCTRL_TEN_MASK;
}

int flag, flagEnterPORTD, flagEnterPORTA, flagRising, flagFalling = 0;

void PIT_IRQHandler() { //every 10 us, also every 1 s
	//Clear Pending IRQ
	NVIC_ClearPendingIRQ(PIT_IRQn);
	//Configure output pin
	if (firstTime == 0) {
		PTD->PDOR |= MASK(TRIGGER_PIN);
		firstTime = 1;
	}
	else if (firstTime == 1) {
		PTD->PDOR &= ~MASK(TRIGGER_PIN);
		firstTime++;
		PIT->CHANNEL[0].TCTRL &= ~PIT_TCTRL_TEN_MASK; //off the timer at channel 0 to stop 10 us pulse
	}
	//Clear Flag
	PIT->CHANNEL[0].TFLG &= PIT_TFLG_TIF_MASK;
}


void PORTD_IRQHandler() {
	NVIC_ClearPendingIRQ(PORTD_IRQn);
	flagEnterPORTD = 1;
	if (PORTD->ISFR & MASK(ECHO_PIN_RISING)) {
		flagRising = 1;
		start_time = PIT->CHANNEL[1].CVAL;
	}
	//Clear INT Flag
	PORTD->ISFR |= MASK(ECHO_PIN_RISING);
}

void PORTA_IRQHandler() {
	NVIC_ClearPendingIRQ(PORTA_IRQn);
	flagEnterPORTA = 1;
	if (PORTA->ISFR & MASK(ECHO_PIN_FALLING)) {
		flagFalling = 1;
		stop_time = PIT->CHANNEL[1].CVAL;
	}
	//Clear INT Flag
	PORTA->ISFR |= MASK(ECHO_PIN_FALLING);
}
	
float getDistance() {
	startUltra();
	float distance = 0;
	float time = 0;
	while (stop_time == 0) {
		int flagecho = flag;
		int flagd = flagEnterPORTD;
		int flaga = flagEnterPORTA;
		int flagr = flagRising;
		int flagf = flagFalling;
		uint32_t time1 = start_time;
		uint32_t time2 = stop_time;
		uint32_t flagCVAL = PIT->CHANNEL[1].CVAL;
		time = (float)(start_time - stop_time + 1) / 10485760; //convert to s
		distance = AIR_SPEED * time; //distance in cm
		int i = 0;		
		flagEnterPORTD = 0;
		flagEnterPORTA = 0;
		flagRising = 0;
		flagFalling = 0;
		
	}

	return distance;
}

