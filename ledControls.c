#include "MKL25Z4.h"                    // Device header
#include "ledControls.h"
#include "cmsis_os2.h"                  // ::CMSIS:RTOS2

#define MASK(x) (1 << (x))
#define LED_NUM 8
#define REAR_LED 9
#define REAR_MOVING 500
#define REAR_STATIONARY 250
//Port C port numbers used for front led strip:
uint8_t frontledstrip[] = {7, 0, 3, 4, 5, 6, 10, 11};

static void delay(volatile uint32_t nof) {
	while(nof!=0) {
		__ASM("NOP");
		nof--;
	}
}

void InitGPIOLed(){
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
	
	//init Rear LEDs
	PORTC->PCR[REAR_LED] &= ~PORT_PCR_MUX_MASK;
	PORTC->PCR[REAR_LED] |= PORT_PCR_MUX(1);
	
	//set port data direction for rear led:
	PTC->PDDR |= MASK(REAR_LED);
}

void offLED(uint8_t number) {
	PTC->PDOR &= ~MASK(number);
}

void onLED(uint8_t number) {
	PTC->PDOR |= MASK(number);
}

void rearLed250() {
	onLED(REAR_LED);
	osDelay(REAR_STATIONARY);
	offLED(REAR_LED);
	osDelay(REAR_STATIONARY);
}

void rearLed500() {
	onLED(REAR_LED);
	osDelay(REAR_MOVING);
	offLED(REAR_LED);
	osDelay(REAR_MOVING);
}

void onAllLED(){
	for (int i = 0; i < LED_NUM; i++) {
		onLED(frontledstrip[i]);
	}
	osDelay(100);
}

void onOffLED(uint8_t number){ 
	onLED(number);
	osDelay(100);
	offLED(number);
}

void runningFrontLED() {
	
	//Off all LEDs first
	for (int i = 0; i < LED_NUM; i++) {
		offLED(frontledstrip[i]);
	}
	
	//Iterate 1 by 1
	for (int i = 0; i < LED_NUM ; i++) {
		onOffLED(frontledstrip[i]);
	}
}

