#include "MKL25Z4.h"                    // Device header
#include "ultrasonic.h"

volatile uint32_t start_time = 0;
volatile uint32_t stop_time = 0;
volatile int flagRising = 0;
volatile int counter = 0;

/*******************
	ULTRASONIC SENSOR
********************/

void InitUltra() {
	SIM_SCGC5 |= SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTE_MASK;
	SIM_SCGC6 |= SIM_SCGC6_TPM2_MASK;
	SIM->SOPT2 &= ~SIM_SOPT2_TPMSRC_MASK;	
	SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1); 	
	//Configure TPM0 to input capture mode
	TPM2->SC &= ~((TPM_SC_CMOD_MASK) | (TPM_SC_PS_MASK) | TPM_SC_CPWMS_MASK);
	TPM2->SC |= TPM_SC_CMOD(1) | TPM_SC_PS(7);
	TPM2->MOD = 375000; //10 s timer, will reset every 1 s
	TPM2_C0SC |= TPM_CnSC_CHIE_MASK | TPM_CnSC_ELSA(1) | TPM_CnSC_ELSB(1);
	//GPIO configuration
	PORTE->PCR[TRIGGER_PIN] &= ~PORT_PCR_MUX_MASK;
	PORTE->PCR[TRIGGER_PIN] |= PORT_PCR_MUX(1);
	PORTB->PCR[ECHO_PIN] &= ~PORT_PCR_MUX_MASK;
	PORTB->PCR[ECHO_PIN] |= PORT_PCR_MUX(3);
	//Set GPIO direction
	PTE->PDDR |= MASK(TRIGGER_PIN);
	PTB->PDDR &= ~MASK(ECHO_PIN);
}

static void delay(volatile uint32_t nof) {
  while(nof!=0) {
    __asm("NOP");
    nof--;
  }
}

void TPM2_IRQHandler() {
	if (counter == 0){
		TPM2_CNT = 0;
	}
	else {
		stop_time = TPM2_C0V;
		TPM2_C0SC &= ~TPM_CnSC_CHIE_MASK; //disable TPM2 interrupt
	}
	counter = 1 - counter;
	//Clear Flag
	TPM2_STATUS |= TPM_STATUS_CH0F_MASK;
}

float getDistance() {
	flagRising = 0;
	stop_time = 0;
	TPM2_C0SC |= TPM_CnSC_CHIE_MASK;
	PTE->PDOR |= MASK(TRIGGER_PIN);
	delay(0x40);
	PTE->PDOR &= ~MASK(TRIGGER_PIN);
	while (stop_time == 0);
	float time = ((float)(stop_time)) / 375000.0;
	float distance = AIR_SPEED * time / 2; 
	return distance;
}
