#include "MKL25Z4.h"                    // Device header
#include "ultrasonic.h"

/*******************
	ULTRASONIC SENSOR
********************/

void InitUltra() {
	SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;
	SIM_SCGC6 |= SIM_SCGC6_TPM0_MASK | SIM_SCGC6_PIT_MASK;
	SIM->SOPT2 &= ~SIM_SOPT2_TPMSRC_MASK;	
	SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1); 	
	//Configure PIT for 10 us pulse generation
	PIT->MCR &= ~PIT_MCR_MDIS_MASK;
	PIT->CHANNEL[0].TCTRL |= PIT_TCTRL_TIE_MASK | PIT_TCTRL_TEN_MASK;
	PIT->CHANNEL[0].LDVAL = 2000;//for 10us pulse
	//Configure TPM0 to input capture mode
	TPM0->SC &= ~((TPM_SC_CMOD_MASK) | (TPM_SC_PS_MASK) | TPM_SC_CPWMS_MASK);
	TPM0->SC |= TPM_SC_CMOD(1) | TPM_SC_PS(7);
	TPM0->MOD = 3750000; //10 s timer, will reset every 1 s
	TPM0_C4SC |= TPM_CnSC_CHIE_MASK | TPM_CnSC_ELSA(1) | TPM_CnSC_ELSB(1);
	//GPIO configuration
	PORTE->PCR[TRIGGER_PIN] &= ~PORT_PCR_MUX_MASK;
	PORTE->PCR[TRIGGER_PIN] |= PORT_PCR_MUX(1);
	PORTE->PCR[ECHO_PIN] &= ~PORT_PCR_MUX_MASK;
	PORTE->PCR[ECHO_PIN] |= PORT_PCR_MUX(3);
	//Set GPIO direction
	PTE->PDDR |= MASK(TRIGGER_PIN);
	PTE->PDDR &= ~MASK(ECHO_PIN);
}

static void delay(volatile uint32_t nof) {
  while(nof!=0) {
    __asm("NOP");
    nof--;
  }
}

volatile int firstTime = 0;
volatile int flagTRIGGER, flagRising, flagFalling = 0;

//Don't delete this... 
void PIT_IRQHandler() { 
	//Clear Pending IRQ
	NVIC_ClearPendingIRQ(PIT_IRQn);
	//Configure output pin
	if (firstTime == 0) {
		flagTRIGGER = 1;
		PTD->PDOR |= MASK(TRIGGER_PIN);
	}
	else if (firstTime == 1) {
		PTD->PDOR &= ~MASK(TRIGGER_PIN);
		PIT->MCR &= ~PIT_TCTRL_TEN_MASK;
		//NVIC_DisableIRQ(PIT_IRQn);
	}
	firstTime = ~firstTime;
	//Clear Flag
	PIT->CHANNEL[0].TFLG &= PIT_TFLG_TIF_MASK;
}

volatile int counter = 0;
volatile uint32_t start_time = 0;
volatile uint32_t stop_time = 0;

void TPM0_IRQHandler() {
	if (counter % 2 == 0) {
		//rising edge
		flagRising = 1;
		start_time = TPM0_C4V;
	}
	else {
		flagFalling = 1;
		stop_time = TPM0_C4V;
	}
	counter = ~counter;
	//Clear Flag
	TPM0_C4SC |= TPM_CnSC_CHF_MASK;
}

float getDistance() {
	//manually on off the trigger pin
	PTE->PDOR |= MASK(TRIGGER_PIN);
	osDelay(1); // delay(0x100);
	PTE->PDOR &= ~MASK(TRIGGER_PIN);
	osDelay(30); // delay(0x3333);//ard 30 ms delay to read start and stop time
	uint32_t start = start_time;
	uint32_t stop = stop_time;
	float time = ((float)(stop_time - start_time)) / 375000.0;
	float distance = AIR_SPEED * time / 2;
	int i = 0;
	return distance;
}