#include "RTE_Components.h"
//#include  CMSIS_device_header
#include "cmsis_os2.h"
#include "MKL25Z4.h"                    // Device header
#include "buzzerControls.h"
#include "motorControls.h"
#include "uart.h"
#include "ledControls.h"
#include <assert.h>

#define QUEUE_MSG_COUNT 1

//To delete if not used in the end.
//Can be useful if Arduino can send packets, but need to parse 32uint data
/*
typedef struct {
	uint8_t cmd;
	uint8_t data;
} myDataPkt;
*/

//Initialize the msgs that control various threads.
const osThreadAttr_t priorityAboveNormal = {
  .priority = osPriorityAboveNormal
};

osMessageQueueId_t motorMsg, buzzerMsg, frontLedMsg, rearLedMsg, completedBuzzerMsg;

osEventFlagsId_t buzzer_flag;

osSemaphoreId_t buzzerSem;

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
		
		rx_data = UART2->D;
		//For easy setting of event flags
		switch(rx_data) {
			case 0x01:
			case 0x02:
			case 0x03:
			case 0x04:
				break;
			case 0x05:
				osEventFlagsSet(buzzer_flag, 0x0001);
				break;
			default:
				break;
		}
	}
	
	if (UART2->S1 & (UART_S1_OR_MASK 
									| UART_S1_NF_MASK 
									| UART_S1_FE_MASK 
									| UART_S1_PF_MASK)) {
		//handle error
										
		//clear flag
	}
}


void app_control_rear_led(void* argument) {
	uint32_t receivedData;
	for (;;) {
		osMessageQueueGet(rearLedMsg, &receivedData, NULL, osWaitForever);
		switch (receivedData) {
			case 0x01:
			case 0x02:
			case 0x03:
			case 0x04:
				rearLed500();
				break;
			default:
				rearLed250();
				break;
		}
	}
	
}



void app_control_front_led(void *argument) {
	uint32_t receivedData;
	for (;;) {
		osMessageQueueGet(frontLedMsg, &receivedData, NULL, osWaitForever);
		switch (receivedData) {
			case 0x01:
			case 0x02:
			case 0x03:
			case 0x04:
				runningFrontLED();
				break;
			default:
				onAllLED();
		}
	}
}


void app_control_motor(void *argument) {
	uint32_t receivedData;
	for (;;) {
		osMessageQueueGet(motorMsg, &receivedData, NULL, osWaitForever);
		switch (receivedData) {
			case 0x01:
				forwards(FULL_SPEED);
				break;
			case 0x02:
				reverse(FULL_SPEED);
				break;
			case 0x03:
				right(FULL_SPEED);
				break;
			case 0x04:
				left(FULL_SPEED);
			default:
				stop_moving();
				break;
		}
	}
}

void app_control_buzzer(void *argument) {
	uint32_t receivedData;
	for (;;) {
		osMessageQueueGet(buzzerMsg, &receivedData, NULL, osWaitForever);
		switch (receivedData) {
			case 0x01:
			case 0x02:
			case 0x03:
			case 0x04:
				playSong(); //Play tune while moving, where 1-4 are movement commands
				break;
			case 0x05: // When the robot finishes moving
				playCompletedSong();
			default:
				break; //Do nothing if not moving
		}
	}
}

void app_control_completed_buzzer(void *argument) {
	uint32_t receivedData;
	for (;;) {
		osMessageQueueGet(completedBuzzerMsg, &receivedData, NULL, osWaitForever);
		osEventFlagsWait(buzzer_flag, 0x0001, osFlagsWaitAny, osWaitForever);
		playCompletedSong();
	}
}

void control_threads(void *argument) {
	uint32_t currCmd;
	while (1) {
		currCmd = rx_data; 
		osMessageQueuePut(completedBuzzerMsg, &currCmd, 1U, 0); //should compete for semaphore and buzzerMsg will not be able to obtain semaphore
		osMessageQueuePut(buzzerMsg, &currCmd, 0U, 0);
		osMessageQueuePut(motorMsg, &currCmd, 0U, 0); //To update with priorities
		osMessageQueuePut(frontLedMsg, &currCmd, 0U, 0);
		osMessageQueuePut(rearLedMsg, &currCmd, 0U, 0);
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
	/*
	PORTB->PCR[RED_LED] |= PORT_PCR_MUX(1);
	PTB->PDDR |= MASK(RED_LED);
	*/
	rx_data = 0;
	
	buzzer_flag = osEventFlagsNew(NULL);
	
	osKernelInitialize();
	
	buzzerSem = osSemaphoreNew(1, 1, NULL); //1 available so can start tune immediately
	
	osThreadNew(control_threads, NULL, NULL); //Initialize the main thread that controls packets
	osThreadNew(app_control_front_led, NULL, NULL);
	osThreadNew(app_control_rear_led, NULL, NULL);
	osThreadNew(app_control_buzzer, NULL, NULL);
	osThreadNew(app_control_motor, NULL, NULL);
	osThreadNew(app_control_completed_buzzer, NULL, NULL);
	
	//Init motor, buzzer and led msgs. Using rx_data instead of myDataPkt as we are not using structures.
	motorMsg = osMessageQueueNew(QUEUE_MSG_COUNT, sizeof(rx_data) , NULL);
	buzzerMsg = osMessageQueueNew(QUEUE_MSG_COUNT, sizeof(rx_data), NULL);
	completedBuzzerMsg = osMessageQueueNew(QUEUE_MSG_COUNT, sizeof(rx_data), NULL);
	frontLedMsg = osMessageQueueNew(QUEUE_MSG_COUNT, sizeof(rx_data), NULL);
	rearLedMsg = osMessageQueueNew(QUEUE_MSG_COUNT, sizeof(rx_data), NULL);


	osKernelStart();
	
	for (;;) { 
		//onAllLED();
		//controlLED(); 
	}
	
}