#include "RTE_Components.h"
//#include  CMSIS_device_header
#include "cmsis_os2.h"
#include "MKL25Z4.h"                    // Device header
#include "buzzerControls.h"
#include "motorControls.h"
#include "uart.h"
#include "ledControls.h"
#include <assert.h>
#include "ultrasonic.h"
#include <stdio.h>

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

osMessageQueueId_t motorMsg, buzzerMsg, frontLedMsg, rearLedMsg, completedBuzzerMsg, selfDrivingMsg;

osEventFlagsId_t buzzer_flag, self_driving_flag, remote_flag;

osSemaphoreId_t buzzerSem;

/*---------------------
FOR DEBUGGING PURPOSES
----------------------*/
#define RED_LED 18

void onRed() {
	PTB->PDOR &= ~MASK(RED_LED);
}

void offRed() {
	PTB->PDOR |= MASK(RED_LED);
}

void InitRedLED() {
	SIM_SCGC5 |= SIM_SCGC5_PORTB_MASK;
	PORTB->PCR[RED_LED] &= ~PORT_PCR_MUX_MASK;
	PORTB->PCR[RED_LED] |= PORT_PCR_MUX(1);
	PTB->PDDR |= MASK(RED_LED);
}
/*---------------------*/

volatile uint32_t rx_data;

void UART2_IRQHandler() {
	
	NVIC_ClearPendingIRQ(UART2_IRQn);
	
	if (UART2->S1 & UART_S1_RDRF_MASK) {
		rx_data = UART2->D;
		
		//For easy setting of event flags
		switch(rx_data) {
			case 0x01:
			case 0x02:
			case 0x03:
			case 0x04:
				break;
			case 0x05:
				osEventFlagsSet(buzzer_flag, 0x0001); //temporary means to switch the buzzer to completed tune
				break;
			case 0x06:
				osEventFlagsSet(buzzer_flag, 0x0000); //temporary means to switch the buzzer back to mario
				break;
			case 0x07: //set flag for self driving mode
				osEventFlagsSet(self_driving_flag, 0x0001);
				osEventFlagsSet(remote_flag, 0x0000);
				break;
			case 0x08: //set flag for remote mode
				osEventFlagsSet(self_driving_flag, 0x0000);
				osEventFlagsSet(remote_flag, 0x0001);
				break;
			default:
				break;
		}
	}
	
	if (UART2->S1 & (UART_S1_OR_MASK | UART_S1_NF_MASK | UART_S1_FE_MASK | UART_S1_PF_MASK)) {
		//handle error
		//clear flag
	}
}

int state = 0;

void app_self_driving(void *argument) {
	uint32_t receivedData;
	for (;;) {
		osMessageQueueGet(selfDrivingMsg, &receivedData, NULL, osWaitForever);
		osEventFlagsWait(self_driving_flag, 0x0001, osFlagsWaitAny, osWaitForever);
		//Reset counter
		//start_time = 0;
		//stop_time = 0;
		TPM0_SC &= ~TPM_SC_CMOD_MASK;
		TPM0->SC |= TPM_SC_CMOD(1);
		TPM0_C4SC |= TPM_CnSC_CHIE_MASK | TPM_CnSC_ELSA(1) | TPM_CnSC_ELSB(1);
		TPM0_CNT = 0;
		PIT->CHANNEL[0].LDVAL = 104;
		//Enable interrupts for PIT and TPM0
		NVIC_EnableIRQ(TPM0_IRQn);
		NVIC_EnableIRQ(PIT_IRQn);
		float distance = getDistance();
		
		switch (state) {
			case 0: //Object Detection State
				if (distance < DISTANCE_THRESHOLD) {
					offRed(); //for visual confirmation that it has been detected
					stop_moving();
					state++; 
				} else {
					onRed(); //Object not yet detected
					forwards(SD_SPEED); 
				}
				break;
			case 1: //To configure turn 90 left
				left(SD_SPEED); 
				osDelay(TURN_DELAY);
				forwards(SD_SPEED); 
			default:
				state = 0;
				break;
		}

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
			case 0x07: //incl self-driving mode
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
			case 0x07: //incl self-driving mode
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
		osMessageQueuePut(selfDrivingMsg, &currCmd, 2U, 0);
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



int main() {
	SystemCoreClockUpdate();
	
	
	InitUART2(BAUD_RATE);
	
	InitPWMMotors();
	InitPWMBuzzer();

	InitGPIOBuzzer();
	InitGPIOMotors();
	InitGPIOLed();
	
	InitUltra();
	
	//Init flags for some threads
	buzzer_flag = osEventFlagsNew(NULL);
	self_driving_flag = osEventFlagsNew(NULL);
	remote_flag = osEventFlagsNew(NULL);
	
	//Default to remote mode
	osEventFlagsSet(remote_flag, 0x0001); 
	
	//Red LED for debugging purposes
	InitRedLED();
	
	rx_data = 0;
	//forwards(SLOW_SPEED);

	/*
	while (1) {
		//Reset counter
		//start_time = 0;
		//stop_time = 0;
		TPM0_SC &= ~TPM_SC_CMOD_MASK;
		TPM0->SC |= TPM_SC_CMOD(1);
		TPM0_C4SC |= TPM_CnSC_CHIE_MASK | TPM_CnSC_ELSA(1) | TPM_CnSC_ELSB(1);
		TPM0_CNT = 0;
		PIT->CHANNEL[0].LDVAL = 104;
		//Enable interrupts for PIT and TPM0
		NVIC_EnableIRQ(TPM0_IRQn);
		NVIC_EnableIRQ(PIT_IRQn);
		float distance = getDistance();
		if (distance < 20) {
			onRed(); //for visual confirmation
		} 
		else{
			offRed();
		}
		int i = 0;
	}
	*/

	
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
	selfDrivingMsg = osMessageQueueNew(QUEUE_MSG_COUNT, sizeof(rx_data), NULL);

	osKernelStart();
	
	for (;;) { 
		//onAllLED();
		//controlLED(); 
	}
	
}