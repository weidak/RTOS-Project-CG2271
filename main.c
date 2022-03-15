#include "MKL25Z4.h"                    // Device header
#include "buzzerControls.h"
#include "motorControls.h"
#include "uart.h"
#include "ledControls.h"


int main() {
	InitPWMMotors();
	//InitPWMBuzzer();
	
	//InitGPIOBuzzer();
	//InitGPIOLed();
	InitGPIOMotors();
}