#include "MKL25Z4.h"                    // Device header
#include "motorControls.h"


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
	//Configure PORTB PCR to TPM mode
	PORTB->PCR[UPPER_LEFT_MOTOR] &= ~PORT_PCR_MUX_MASK;
	PORTB->PCR[UPPER_LEFT_MOTOR] |= PORT_PCR_MUX(3); //TPM1
	PORTB->PCR[UPPER_RIGHT_MOTOR] &= ~PORT_PCR_MUX_MASK;
	PORTB->PCR[UPPER_RIGHT_MOTOR] |= PORT_PCR_MUX(3); //TPM1
	PORTB->PCR[BOTTOM_LEFT_MOTOR] &= ~PORT_PCR_MUX_MASK;
	PORTB->PCR[BOTTOM_LEFT_MOTOR] |= PORT_PCR_MUX(3);	//TPM2
	PORTB->PCR[BOTTOM_RIGHT_MOTOR] &= ~PORT_PCR_MUX_MASK;
	PORTB->PCR[BOTTOM_RIGHT_MOTOR] |= PORT_PCR_MUX(3); //TPM2
}

//TODO: fix up the speed values, set up motor control
void fowards(uint32_t speed){
	UPPER_LEFT_WHEEL = speed;
	UPPER_RIGHT_WHEEL = speed;
	BOTTOM_LEFT_WHEEL = speed;
	BOTTOM_RIGHT_WHEEL = speed;
}

//TODO: Put negative potential difference across AIN1 and AIN2
void reverse(uint32_t speed) {
	UPPER_LEFT_WHEEL = -speed;
	UPPER_RIGHT_WHEEL = -speed;
	BOTTOM_LEFT_WHEEL = -speed;
	BOTTOM_RIGHT_WHEEL = -speed;
}


void left(uint32_t speed) {
	UPPER_LEFT_WHEEL = 0;
	UPPER_RIGHT_WHEEL = speed;
	BOTTOM_LEFT_WHEEL = 0;
	BOTTOM_RIGHT_WHEEL = speed;
}

void left90(uint32_t speed) {
	
}

void right(uint32_t speed) {
	UPPER_LEFT_WHEEL = speed;
	UPPER_RIGHT_WHEEL = 0;
	BOTTOM_LEFT_WHEEL = speed;
	BOTTOM_RIGHT_WHEEL = 0;
}

void right90(uint32_t speed) {
	
}

void stop_moving() {
	UPPER_LEFT_WHEEL = 0;
	UPPER_RIGHT_WHEEL = 0;
	BOTTOM_LEFT_WHEEL = 0;
	BOTTOM_RIGHT_WHEEL = 0;
}

static void delay(volatile uint32_t nof) {
  while(nof!=0) {
    __asm("NOP");
    nof--;
  }
}

/*
int main(void) {
	SystemCoreClockUpdate();
	initPWM();
	initGPIO();
	while (1) {
		go_forward(3750);
		delay(0x88000);
		
		go_backward(3750);
		delay(0x88000);
		
		turn_left(3750);
		delay(0x88000);
		
		turn_right(3750);
		delay(0x88000);
		
		stop_moving();
		delay(0x88000);
	}
}
*/