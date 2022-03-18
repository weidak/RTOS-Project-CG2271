#include "RTE_Components.h"
//#include  CMSIS_device_header
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

volatile uint32_t rx_data;

void UART2_IRQHandler() {
	
	NVIC_ClearPendingIRQ(UART2_IRQn);
	
	/* Don't need to transmit, only receive from controller? 
	if (UART2->S1 & UART_S1_TDRE_MASK) {
		//Space for more characters:
		if (!isEmpty(&tx_q)) UART2->D = dequeue(&tx_q);
		else UART2->C2 &= ~UART_C2_TIE_MASK;
	}
	*/
	if (UART2->S1 & UART_S1_RDRF_MASK) {
		//receives a character
		
		//if (!isFull(&rx_q)) enqueue(&rx_q, UART2->D);
		//else while (1); //queue is full, stay in interrupt?
		rx_data = UART2->D;
	}
	
	if (UART2->S1 & (UART_S1_OR_MASK 
									| UART_S1_NF_MASK 
									| UART_S1_FE_MASK 
									| UART_S1_PF_MASK)) {
		//handle error
										
		//clear flag
	}
}

static void delay(volatile uint32_t nof) {
  while(nof!=0) {
    __asm("NOP");
    nof--;
  }
}

#define RED_LED 18

void onRed() {
	PTB->PDOR &= ~MASK(RED_LED);
}

void offRed() {
	PTB->PDOR |= MASK(RED_LED);
}

int main() {
	SystemCoreClockUpdate();
	initGPIOLed();
	
	InitUART2(BAUD_RATE);
	InitPWMMotors();
	InitPWMBuzzer();
	
	InitGPIOBuzzer();
	InitGPIOMotors();
	PORTB->PCR[RED_LED] |= PORT_PCR_MUX(1);
	PTB->PDDR |= MASK(RED_LED);
	rx_data = 0;
	while (1) {
		if (rx_data == 0x31) {
			//F
			onRed();
			forwards(HALF_SPEED);
		}
		else if (rx_data == 0x32) {
			reverse(HALF_SPEED);
		}
		else if (rx_data == 0x33) {
			left(HALF_SPEED);
		}
		else if (rx_data == 0x34) {
			right(HALF_SPEED);
		}
		//delay(0x800000);
		offRed();
		//stop_moving();
	}
	
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