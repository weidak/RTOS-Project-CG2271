#include "MKL25Z4.h"                    // Device header

#define UPPER_LEFT_MOTOR 0		//PORTB PIN 0
#define UPPER_RIGHT_MOTOR 2		//PORTB PIN 1
#define BOTTOM_LEFT_MOTOR 1	  //PORTB PIN 2
#define BOTTOM_RIGHT_MOTOR 3  //PORTB PIN 3

#define UPPER_LEFT_WHEEL TPM2_C1V
#define UPPER_RIGHT_WHEEL TPM1_C1V
#define BOTTOM_LEFT_WHEEL TPM2_C0V
#define BOTTOM_RIGHT_WHEEL TPM1_C0V

#define FULL_SPEED 7501
#define HALF_SPEED 3500

#define MASK(x) (1UL << x)

void init_PWM() {
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

void init_GPIO_with_PWM(){
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
	init_PWM();
}

void go_forward(int speed){
	UPPER_LEFT_WHEEL = speed;
	UPPER_RIGHT_WHEEL = speed;
	BOTTOM_LEFT_WHEEL = speed;
	BOTTOM_RIGHT_WHEEL = speed;
}

void go_backward(int speed) {
	UPPER_LEFT_WHEEL = -speed;
	UPPER_RIGHT_WHEEL = -speed;
	BOTTOM_LEFT_WHEEL = -speed;
	BOTTOM_RIGHT_WHEEL = -speed;
}

void turn_left(int speed) {
	UPPER_LEFT_WHEEL = 0;
	UPPER_RIGHT_WHEEL = speed;
	BOTTOM_LEFT_WHEEL = 0;
	BOTTOM_RIGHT_WHEEL = speed;
}

void turn_right(int speed) {
	UPPER_LEFT_WHEEL = speed;
	UPPER_RIGHT_WHEEL = 0;
	BOTTOM_LEFT_WHEEL = speed;
	BOTTOM_RIGHT_WHEEL = 0;
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

int main(void) {
	SystemCoreClockUpdate();
	init_GPIO_with_PWM();
	while (1) {
		/*go_forward(3750);
		delay(0x88000);
		
		go_backward(3750);
		delay(0x88000);
		
		turn_left(3750);
		delay(0x88000);*/
		
		turn_right(3750);
		delay(0x88000);
		
		/*stop_moving();
		delay(0x88000);*/
	}
}