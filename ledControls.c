#include "MKL25Z4.h"                    // Device header
#include "ledControls.h"
#include "cmsis_os2.h"                  // ::CMSIS:RTOS2

#define MASK(x) (1 << (x))
#define LED_NUM 8
//Port C port numbers used for front led strip:
uint8_t frontledstrip[] = {7, 0, 3, 4, 5, 6, 10, 11};

static void delay(volatile uint32_t nof) {
	while(nof!=0) {
		__ASM("NOP");
		nof--;
	}
}

void initGPIOLed(){
	//Enable clock gating for portc
	SIM_SCGC5 |= SIM_SCGC5_PORTC_MASK;
	
	//configure for mode 1 for gpio for all the ports
	for (int i = 0; i < LED_NUM; i++) {
		PORTC->PCR[frontledstrip[i]] &= ~PORT_PCR_MUX_MASK;
		PORTC->PCR[frontledstrip[i]] |= PORT_PCR_MUX(1);
	}
	
	//sets port data direction for each of the pin
	for (int i = 0; i < LED_NUM; i++) {
		PTC->PDDR |= MASK(frontledstrip[i]);
	}
}

void offLED() {
	for (int i = 0; i < LED_NUM; i++) {
		PTC->PDOR &= ~MASK(frontledstrip[i]);
	}
}

void onLED(uint8_t number) {
	PTC->PDOR |= MASK(number);
}

void onOffLED(uint8_t number){ 
	onLED(number);
	osDelay(1000);//delay(0xFFFF);
	offLED();
}

void onAllLED(){
	for (int i = 0; i < LED_NUM; i++) {
		onLED(frontledstrip[i]);
	}
}


void controlLED() {
	for (int i = 0; i < LED_NUM; i++) onOffLED(frontledstrip[i]);
}

