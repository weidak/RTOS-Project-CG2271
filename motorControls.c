#include "MKL25Z4.h"                    // Device header
#include "motorControls.h"
#include "ultrasonic.h"
#include <stdarg.h>

void InitPWMMotors(){
	//Enable clock source for timer 1 and 2
	SIM_SCGC6 |= SIM_SCGC6_TPM1_MASK | SIM_SCGC6_TPM2_MASK;
	//Select clock source for TPM1 and TPM2
	SIM_SOPT2 &= ~SIM_SOPT2_TPMSRC_MASK;
	SIM_SOPT2 |= SIM_SOPT2_TPMSRC(1);
	//Set prescaler and internal clock
	TPM1->SC &= ~(TPM_SC_CMOD_MASK | TPM_SC_PS_MASK);
	TPM1->SC |= TPM_SC_CMOD(1) | TPM_SC_PS(7);
	TPM1->SC &= ~TPM_SC_CPWMS_MASK;
	TPM2->SC &= ~(TPM_SC_CMOD_MASK | TPM_SC_PS_MASK);
	TPM2->SC |= TPM_SC_CMOD(1) | TPM_SC_PS(7);
	TPM2->SC &= ~TPM_SC_CPWMS_MASK;
	//Set MOD value
	TPM1->MOD = 7500; //fPWM = 50 Hz
	TPM2->MOD = 7500; //fPWM = 50 Hz
	//Set to edge-aligned with high-true pulses for PORTB Pin 0 and PORTB Pin 2
	TPM1_C0SC &= ~(TPM_CnSC_ELSA_MASK | TPM_CnSC_ELSB_MASK | TPM_CnSC_MSA_MASK | TPM_CnSC_MSB_MASK);
	TPM1_C0SC |= TPM_CnSC_ELSB(1) | TPM_CnSC_MSB(1);	
	TPM2_C0SC &= ~(TPM_CnSC_ELSA_MASK | TPM_CnSC_ELSB_MASK | TPM_CnSC_MSA_MASK | TPM_CnSC_MSB_MASK);
	TPM2_C0SC |= TPM_CnSC_ELSB(1) | TPM_CnSC_MSB(1); 
	//TPM1 C1 and TPM2 C2 is for PORTB Pin 1 and PORTB Pin 3
	TPM1_C1SC &= ~(TPM_CnSC_ELSA_MASK | TPM_CnSC_ELSB_MASK | TPM_CnSC_MSA_MASK | TPM_CnSC_MSB_MASK);
	TPM1_C1SC |= TPM_CnSC_ELSB(1) | TPM_CnSC_MSB(1);
	TPM2_C1SC &= ~(TPM_CnSC_ELSA_MASK | TPM_CnSC_ELSB_MASK | TPM_CnSC_MSA_MASK | TPM_CnSC_MSB_MASK);
	TPM2_C1SC |= TPM_CnSC_ELSB(1) | TPM_CnSC_MSB(1);
}

void InitGPIOMotors(){
	//Enable clock source for PORTB GPIO
	SIM_SCGC5 |= SIM_SCGC5_PORTB_MASK;
	//Set each pin to timer module to allow PWM mode
	PORTB->PCR[FORW_LEFT_MOTORS] &= ~PORT_PCR_MUX_MASK; //PORTB PIN0
	PORTB->PCR[FORW_LEFT_MOTORS] |= PORT_PCR_MUX(3); 
	PORTA->PCR[REV_LEFT_MOTORS] &= ~PORT_PCR_MUX_MASK; //B1
	PORTA->PCR[REV_LEFT_MOTORS] |= PORT_PCR_MUX(3);
	PORTB->PCR[FORW_RIGHT_MOTORS] &= ~PORT_PCR_MUX_MASK; //B2
	PORTB->PCR[FORW_RIGHT_MOTORS] |= PORT_PCR_MUX(3);
	PORTB->PCR[REV_RIGHT_MOTORS] &= ~PORT_PCR_MUX_MASK; //B3
	PORTB->PCR[REV_RIGHT_MOTORS] |= PORT_PCR_MUX(3);
}

//TODO: fix up the speed values for forward moving
void forwards(uint32_t speed) {
	//change the duty cycle according to the speed desired
	FORW_LEFT_WHEELS = speed; //TPM1->C0V
	FORW_RIGHT_WHEELS = speed; //TPM2->C0V
	REV_LEFT_WHEELS = 0;
	REV_RIGHT_WHEELS = 0;
}

//TODO: fix up the speed values for reverse moving
void reverse(uint32_t speed) {
	FORW_LEFT_WHEELS = 0;
	FORW_RIGHT_WHEELS = 0;
	REV_LEFT_WHEELS = speed;
	REV_RIGHT_WHEELS = speed;
}

//TODO: fix up the speed values for turning left while moving 
void left(uint32_t speed) {
	FORW_LEFT_WHEELS = 0;
	FORW_RIGHT_WHEELS = speed*2;
	REV_LEFT_WHEELS = 0;
	REV_RIGHT_WHEELS = 0;
}

void left_stationary(uint32_t speed) {
	FORW_LEFT_WHEELS = 0;
	FORW_RIGHT_WHEELS = speed;
	REV_LEFT_WHEELS = 0;
	REV_RIGHT_WHEELS = speed;
}

//TODO: fix up the speed values for left 90 degree
void left45(uint32_t speed) {
	left_stationary(speed);
	osDelay(DELAY_LEFT_TURN);
	stop_moving();
	osDelay(DELAY_STOP);
	
}

//TODO: fix up the speed values for turning right while moving
void right(uint32_t speed) {
	FORW_LEFT_WHEELS = speed*2;
	FORW_RIGHT_WHEELS = 0;
	REV_LEFT_WHEELS = 0;
	REV_RIGHT_WHEELS = 0;
}

void right_stationary(uint32_t speed) {
	FORW_LEFT_WHEELS = speed;
	FORW_RIGHT_WHEELS = 0;
	REV_LEFT_WHEELS = speed;
	REV_RIGHT_WHEELS = 0;
}

//TODO: fix up the speed values for right 90 degree
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

void move(uint32_t cmd, uint32_t speed) {
	osSemaphoreAcquire(movementSem, osWaitForever);
	switch (cmd){
		case 0x01:
			forwards(speed);
			break;
		case 0x02:
			reverse(speed);
			break;
		case 0x03:
			right(speed);
			break;
		case 0x04:
			left(speed);
			break;
		case 0x05:
			left45(speed);
			break;
		case 0x06:
			right90(speed);
			break;
		case 0x07:
			//self-driving
			break;
		case 0x08:
			stop_moving();
			break;
		default:
			//play buzzer
			break;
	}
	osSemaphoreRelease(movementSem);
}

static void delay(volatile uint32_t nof) {
  while(nof!=0) {
    __asm("NOP");
    nof--;
  }
}
