#include "MKL25Z4.h"                    // Device header
#include "motorControls.h"
#include "ultrasonic.h"
#include <stdarg.h>

void InitPWMMotors() {
	//Enable clock source for timer 1 and 2
	SIM_SCGC6 |= SIM_SCGC6_TPM0_MASK;
	//Select clock source for TPM1 and TPM2
	SIM_SOPT2 &= ~SIM_SOPT2_TPMSRC_MASK;
	SIM_SOPT2 |= SIM_SOPT2_TPMSRC(1);
	//Set prescaler and internal clock
	TPM0->SC &= ~(TPM_SC_CMOD_MASK | TPM_SC_PS_MASK);
	TPM0->SC |= TPM_SC_CMOD(1) | TPM_SC_PS(7);
	TPM0->SC &= ~TPM_SC_CPWMS_MASK;
	//Set MOD value
	TPM0->MOD = 7500; //fPWM = 50 Hz
	//Set to edge-aligned with high-true pulses for PORTB Pin 0 and PORTB Pin 2
	TPM0_C0SC &= ~(TPM_CnSC_ELSA_MASK | TPM_CnSC_ELSB_MASK | TPM_CnSC_MSA_MASK | TPM_CnSC_MSB_MASK);
	TPM0_C0SC |= TPM_CnSC_ELSB(1) | TPM_CnSC_MSB(1);	
	TPM0_C1SC &= ~(TPM_CnSC_ELSA_MASK | TPM_CnSC_ELSB_MASK | TPM_CnSC_MSA_MASK | TPM_CnSC_MSB_MASK);
	TPM0_C1SC |= TPM_CnSC_ELSB(1) | TPM_CnSC_MSB(1); 
	//TPM1 C1 and TPM2 C2 is for PORTB Pin 1 and PORTB Pin 3
	TPM0_C2SC &= ~(TPM_CnSC_ELSA_MASK | TPM_CnSC_ELSB_MASK | TPM_CnSC_MSA_MASK | TPM_CnSC_MSB_MASK);
	TPM0_C2SC |= TPM_CnSC_ELSB(1) | TPM_CnSC_MSB(1);
	TPM0_C3SC &= ~(TPM_CnSC_ELSA_MASK | TPM_CnSC_ELSB_MASK | TPM_CnSC_MSA_MASK | TPM_CnSC_MSB_MASK);
	TPM0_C3SC |= TPM_CnSC_ELSB(1) | TPM_CnSC_MSB(1);
}

void InitGPIOMotors(){
	//Enable clock source for PORTB GPIO
	SIM_SCGC5 |= SIM_SCGC5_PORTD_MASK;
	//Set each pin to timer module to allow PWM mode
	PORTD->PCR[FORW_LEFT_MOTORS] &= ~PORT_PCR_MUX_MASK; //PORTB PIN0
	PORTD->PCR[FORW_LEFT_MOTORS] |= PORT_PCR_MUX(4); 
	PORTD->PCR[REV_LEFT_MOTORS] &= ~PORT_PCR_MUX_MASK; //B1
	PORTD->PCR[REV_LEFT_MOTORS] |= PORT_PCR_MUX(4);
	PORTD->PCR[FORW_RIGHT_MOTORS] &= ~PORT_PCR_MUX_MASK; //B2
	PORTD->PCR[FORW_RIGHT_MOTORS] |= PORT_PCR_MUX(4);
	PORTD->PCR[REV_RIGHT_MOTORS] &= ~PORT_PCR_MUX_MASK; //B3
	PORTD->PCR[REV_RIGHT_MOTORS] |= PORT_PCR_MUX(4);
}

void forwards(uint32_t speed) {
	//change the duty cycle according to the speed desired
	FORW_LEFT_WHEELS = speed; 
	FORW_RIGHT_WHEELS = speed; 
	REV_LEFT_WHEELS = 0;
	REV_RIGHT_WHEELS = 0;
}

void reverse(uint32_t speed) {
	FORW_LEFT_WHEELS = 0;
	FORW_RIGHT_WHEELS = 0;
	REV_LEFT_WHEELS = speed;
	REV_RIGHT_WHEELS = speed;
}

void left(uint32_t speed) {
	FORW_LEFT_WHEELS = 0;
	FORW_RIGHT_WHEELS = speed;
	REV_LEFT_WHEELS = 0;
	REV_RIGHT_WHEELS = 0;
}

void left_stationary(uint32_t speed) {
	FORW_LEFT_WHEELS = 0;
	FORW_RIGHT_WHEELS = speed;
	REV_LEFT_WHEELS = speed;
	REV_RIGHT_WHEELS = 0;
}

void left45(uint32_t speed) {
	left_stationary(speed);
	osDelay(DELAY_LEFT_TURN);
	stop_moving();
	osDelay(DELAY_STOP);
	
}

void right(uint32_t speed) {
	FORW_LEFT_WHEELS = speed;
	FORW_RIGHT_WHEELS = 0;
	REV_LEFT_WHEELS = 0;
	REV_RIGHT_WHEELS = 0;
}

void right_stationary(uint32_t speed) {
	FORW_LEFT_WHEELS = speed;
	FORW_RIGHT_WHEELS = 0;
	REV_LEFT_WHEELS = 0;
	REV_RIGHT_WHEELS = speed;
}

void right90(uint32_t speed) {
	right_stationary(speed);
	osDelay(DELAY_RIGHT_TURN);
	stop_moving();
	osDelay(DELAY_STOP);
}

void stop_moving() {
	FORW_LEFT_WHEELS = 0;
	FORW_RIGHT_WHEELS = 0;
	REV_LEFT_WHEELS = 0;
	REV_RIGHT_WHEELS = 0;
}

