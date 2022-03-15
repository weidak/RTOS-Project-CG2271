#include "MKL25Z4.h"                    // Device header
#include "buzzerControls.h"
#include "motorControls.h"
#include "uart.h"
#include "ledControls.h"

//TODO: Test motor controls and interrupts? 
//TODO: See if UART is able to receive commands from esp32 board with led strips bah
int main() {
	SystemCoreClockUpdate();
	
	InitPWMMotors();
	InitUART2(BAUD_RATE);
	//InitPWMBuzzer();
	
	//InitGPIOBuzzer();
	//InitGPIOLed();
	InitGPIOMotors();
	
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
}