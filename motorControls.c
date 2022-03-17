#include "MKL25Z4.h"                    // Device header
#include "motorControls.h"
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
	//Configure PORTB PCR to TPM mode
	
	//Configure forwards and reverse for upper left motor
	//F
	PORTB->PCR[FORW_UPPER_LEFT_MOTOR] &= ~PORT_PCR_MUX_MASK;
	PORTB->PCR[FORW_UPPER_LEFT_MOTOR] |= PORT_PCR_MUX(3); //TPM1
	//R
	PORTB->PCR[REV_UPPER_LEFT_MOTOR] &= ~PORT_PCR_MUX_MASK;
	PORTB->PCR[REV_UPPER_LEFT_MOTOR] |= PORT_PCR_MUX(3); //TPM1
	
	//Configure forwards and reverse for upper right motor
	//F
	PORTB->PCR[FORW_UPPER_RIGHT_MOTOR] &= ~PORT_PCR_MUX_MASK;
	PORTB->PCR[FORW_UPPER_RIGHT_MOTOR] |= PORT_PCR_MUX(3); //TPM1
	//R
	PORTB->PCR[REV_UPPER_RIGHT_MOTOR] &= ~PORT_PCR_MUX_MASK;
	PORTB->PCR[REV_UPPER_RIGHT_MOTOR] |= PORT_PCR_MUX(3); //TPM1

	//Configure forwards and reverse for bottom left motor
	//F
	PORTB->PCR[FORW_BOTTOM_LEFT_MOTOR] &= ~PORT_PCR_MUX_MASK;
	PORTB->PCR[FORW_BOTTOM_LEFT_MOTOR] |= PORT_PCR_MUX(3);	//TPM2
	//R
	PORTB->PCR[REV_BOTTOM_LEFT_MOTOR] &= ~PORT_PCR_MUX_MASK;
	PORTB->PCR[REV_BOTTOM_LEFT_MOTOR] |= PORT_PCR_MUX(3);	//TPM2
	
	//Configure forwards and reverse for bottom right motor
	//F
	PORTB->PCR[FORW_BOTTOM_RIGHT_MOTOR] &= ~PORT_PCR_MUX_MASK;
	PORTB->PCR[FORW_BOTTOM_RIGHT_MOTOR] |= PORT_PCR_MUX(3); //TPM2
	//R
	PORTB->PCR[REV_BOTTOM_RIGHT_MOTOR] &= ~PORT_PCR_MUX_MASK;
	PORTB->PCR[REV_BOTTOM_RIGHT_MOTOR] |= PORT_PCR_MUX(3); //TPM2
}

void enableForwMotorPin(int n,...){
	va_list pins;
	va_start(pins, n);
	for (int i = 0; i < n; i++) {
		int pin = va_arg(pins, int);
		PORTB->PCR[pin] |= PORT_PCR_MUX(3);
	}
	va_end(pins);
}

void enableRevMotorPin(int n,...){
	va_list pins;
	va_start(pins, n);
	for (int i = 0; i < n; i++) {
		int pin = va_arg(pins, int);
		PORTA->PCR[pin] |= PORT_PCR_MUX(3);
	}
	va_end(pins);
}

void disableAllPins(){
	PORTB->PCR[FORW_UPPER_LEFT_MOTOR] |= PORT_PCR_MUX(0);
	PORTB->PCR[REV_UPPER_LEFT_MOTOR] |= PORT_PCR_MUX(0);
	PORTB->PCR[FORW_UPPER_RIGHT_MOTOR] |= PORT_PCR_MUX(0);
	PORTB->PCR[REV_UPPER_RIGHT_MOTOR] |= PORT_PCR_MUX(0);
	PORTB->PCR[FORW_BOTTOM_LEFT_MOTOR] |= PORT_PCR_MUX(0);
	PORTB->PCR[REV_BOTTOM_LEFT_MOTOR] |= PORT_PCR_MUX(0);
	PORTB->PCR[FORW_BOTTOM_RIGHT_MOTOR] |= PORT_PCR_MUX(0);
	PORTB->PCR[REV_BOTTOM_RIGHT_MOTOR] |= PORT_PCR_MUX(0);
}

void assignDutyCycle(speed){
	UPPER_LEFT_WHEEL = speed;
	UPPER_RIGHT_WHEEL = speed;
	BOTTOM_LEFT_WHEEL = speed;
	BOTTOM_RIGHT_WHEEL = speed;
}

//TODO: fix up the speed values, set up motor control
void forwards(uint32_t speed) {
	//disable all reverse motors and enable all forward motors
	disableAllPins();
	enableForwMotorPin(4, 
			FORW_UPPER_LEFT_MOTOR, 
			FORW_BOTTOM_LEFT_MOTOR, 
			FORW_UPPER_RIGHT_MOTOR, 
			FORW_BOTTOM_RIGHT_MOTOR);
	//change the duty cycle according to the speed desired
	assignDutyCycle(speed);
}

//TODO: Put negative potential difference across AIN1 and AIN2
void reverse(uint32_t speed) {
	//disable all forward motors and enable all reverse motors
	disableAllPins();
	enableRevMotorPin(4, REV_UPPER_LEFT_MOTOR, REV_BOTTOM_LEFT_MOTOR, REV_UPPER_RIGHT_MOTOR, REV_BOTTOM_RIGHT_MOTOR);
	//change the duty cycle according to the speed desired 
	assignDutyCycle(speed);
}

void left(uint32_t speed) {
	//disable reverse motors and enable forward motors
	disableAllPins();
	enableForwMotorPin(2, FORW_UPPER_RIGHT_MOTOR, FORW_BOTTOM_RIGHT_MOTOR);
	enableRevMotorPin(2, REV_UPPER_LEFT_MOTOR, REV_BOTTOM_LEFT_MOTOR);
	assignDutyCycle(speed);
}

void left90() {
	disableAllPins();
	enableForwMotorPin(2, FORW_UPPER_RIGHT_MOTOR, FORW_BOTTOM_RIGHT_MOTOR);
	enableRevMotorPin(2, REV_UPPER_LEFT_MOTOR, REV_BOTTOM_LEFT_MOTOR);
}

void right(uint32_t speed) {
	disableAllPins();
	enableForwMotorPin(2, FORW_UPPER_LEFT_MOTOR, FORW_BOTTOM_RIGHT_MOTOR);
	enableRevMotorPin(2, REV_UPPER_RIGHT_MOTOR, REV_BOTTOM_RIGHT_MOTOR);
	assignDutyCycle(speed);
}

void right90() {
	disableAllPins();
	enableForwMotorPin(2, FORW_UPPER_LEFT_MOTOR, FORW_BOTTOM_RIGHT_MOTOR);
	enableRevMotorPin(2, REV_UPPER_RIGHT_MOTOR, REV_BOTTOM_RIGHT_MOTOR);
	
}

void stop_moving() {
	UPPER_LEFT_WHEEL = 0;
	BOTTOM_LEFT_WHEEL = 0;
	UPPER_RIGHT_WHEEL = 0;
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
