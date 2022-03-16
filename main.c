#include "RTE_Components.h"
#include  CMSIS_device_header
#include "cmsis_os2.h"
#include "MKL25Z4.h"                    // Device header
#include "buzzerControls.h"
#include "motorControls.h"
#include "uart.h"
#include "ledControls.h"

//TODO: Test motor controls and interrupts? 
//TODO: See if UART is able to receive commands from esp32 board with led strips bah

void app_control_led(void *argument) {
	for (;;) {
		controlLED();
	}
}

int main() {
	SystemCoreClockUpdate();
	initGPIOLed();
	osKernelInitialize();
	osThreadNew(app_control_led, NULL, NULL);
	osKernelStart();
	
	for (;;) { 
		//onAllLED();
		//controlLED(); 
	}
	
	
	//InitUART2(BAUD_RATE);
	//InitPWMMotors();
	//InitPWMBuzzer();
	
	//InitGPIOBuzzer();
	//InitGPIOLed();
	//InitGPIOMotors();
	
	/*
	while (1) {
		forwards(3750);
		delay(0x88000);
		
		reverse(3750);
		delay(0x88000);
		
		left(3750);
		delay(0x88000);
		
		right(3750);
		delay(0x88000);
		
		stop();
		delay(0x88000);
	}
	*/
	return 0;
}