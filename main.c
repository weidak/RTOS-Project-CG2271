#include "RTE_Components.h"
#include  CMSIS_device_header
#include "cmsis_os2.h"
#include "MKL25Z4.h"                    // Device header
#include "buzzerControls.h"
#include "motorControls.h"
#include "uart.h"
#include "ledControls.h"

osMutexId_t myMutex;

//TODO: Test motor controls and interrupts? 
//TODO: See if UART is able to receive commands from esp32 board with led strips bah

void app_control_front_led(void *argument) {
	for (;;) {
		//osMutexAcquire(myMutex, osWaitForever);
		runningFrontLED();
		//osMutexRelease(myMutex);
	}
}

void app_control_rear_led(void *argument) {
	for(;;) {
		//osMutexAcquire(myMutex, osWaitForever);
		rearLed250();
		//rearLed500();
		//osMutexRelease(myMutex);
	}
}

void app_control_motor(void *argument) {
	
	for (;;) {forwards(3500);}
}

void app_control_buzzer(void *argument) {
	
	for (;;) {
		playSong();
	}
}


int main() {
	SystemCoreClockUpdate();
	initGPIOLed();
	
	//InitUART2(BAUD_RATE);
	InitPWMMotors();
	InitPWMBuzzer();
	
	InitGPIOBuzzer();
	InitGPIOMotors();
	
	
	osKernelInitialize();
	//myMutex = osMutexNew(NULL);
	//osThreadNew(app_control_motor, NULL, NULL);
	osThreadNew(app_control_buzzer, NULL, NULL);
	//osThreadNew(app_control_rear_led, NULL, NULL);
	//osThreadNew(app_control_front_led, NULL, NULL);

	osKernelStart();
	
	for (;;) { 
		//onAllLED();
		//controlLED(); 
	}
	
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