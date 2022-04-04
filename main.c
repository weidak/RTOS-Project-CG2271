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

#define CMD_FORWARD 0x01
#define CMD_REVERSE 0x02
#define CMD_RIGHT 0x03
#define CMD_LEFT 0x04
#define CMD_LEFT45 0x05
#define CMD_RIGHT90 0x06
#define CMD_SELF_DRIVING 0x07
#define CMD_STOP 0x08
#define CMD_BUZZER 0x09

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

const osThreadAttr_t priorityAboveNormal2 = {
	.priority = osPriorityAboveNormal2
};

const osThreadAttr_t priorityAboveNormal1 = {
	.priority = osPriorityAboveNormal1
};

osMessageQueueId_t motorMsg, buzzerMsg, frontLedMsg, rearLedMsg, completedBuzzerMsg, selfDrivingMsg;

osEventFlagsId_t buzzerFlag, selfDrivingFlag, remoteFlag, movementFlag, stopFlag;

osSemaphoreId_t buzzerSem, movementSem;

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

int isMoving() {
	if (rx_data == CMD_
}

volatile uint32_t rx_data;

void UART2_IRQHandler() {
	
	NVIC_ClearPendingIRQ(UART2_IRQn);
	
	if (UART2->S1 & UART_S1_RDRF_MASK) {
		rx_data = UART2->D; //Handling is performed in control threads
	}
	
	if (UART2->S1 & (UART_S1_OR_MASK | UART_S1_NF_MASK | UART_S1_FE_MASK | UART_S1_PF_MASK)) {
		//handle error
		//clear flag
	}
}

volatile int state = 0;
volatile float distance;

void app_self_driving(void *argument) {
	for (;;) {
		osEventFlagsWait(selfDrivingFlag, 0x0001, osFlagsWaitAny, osWaitForever);
		//Reset counter
		//start_time = 0;
		//stop_time = 0;
		while (1) {
			distance = DISTANCE_THRESHOLD;
			TPM0_SC &= ~TPM_SC_CMOD_MASK;
			TPM0->SC |= TPM_SC_CMOD(1);
			TPM0_C4SC |= TPM_CnSC_CHIE_MASK | TPM_CnSC_ELSA(1) | TPM_CnSC_ELSB(1);
			TPM0_CNT = 0;
			PIT->CHANNEL[0].LDVAL = 104;
			//Enable interrupts for PIT and TPM0
			NVIC_EnableIRQ(TPM0_IRQn);
			NVIC_EnableIRQ(PIT_IRQn);
			distance = getDistance();
			float currDist = distance;
			int i = 0;
		//distance = 21;
			//if (distance > 20) forwards(HALF_SPEED);
			/*if (distance < 20) {
			onRed(); //for visual confirmation
			} 
			else{
				offRed();
			}*/
			while (distance >= DISTANCE_THRESHOLD) {
					offRed();
					distance = getDistance();
					move(CMD_FORWARD, SD_SPEED); 
			}
			onRed();
			move(CMD_STOP, SD_SPEED); 
			osDelay(DELAY_STOP);
			move(CMD_LEFT45, SD_SPEED);   //45 degree 
			osDelay(DELAY_LEFT_TURN);

			move(CMD_FORWARD, SD_SPEED); 
			osDelay(DELAY_STRAIGHT);
			move(CMD_STOP, SD_SPEED);   
			osDelay(DELAY_STOP);
			move(CMD_RIGHT90, SD_SPEED); //90 degree
			osDelay(DELAY_RIGHT_TURN);
			
			move(CMD_FORWARD, SD_SPEED);  
			osDelay(DELAY_STRAIGHT);
			move(CMD_STOP, SD_SPEED);  
			osDelay(DELAY_STOP);
			move(CMD_RIGHT90, SD_SPEED); //90 degree
			osDelay(DELAY_RIGHT_TURN);
			
			move(CMD_FORWARD, SD_SPEED); 
			osDelay(DELAY_STRAIGHT);
			move(CMD_STOP, SD_SPEED);  
			osDelay(DELAY_STOP);
			move(CMD_RIGHT90, SD_SPEED);  //90 degree
			osDelay(DELAY_RIGHT_TURN);
			
			move(CMD_FORWARD, SD_SPEED); 
			osDelay(DELAY_STRAIGHT);
			move(CMD_STOP, SD_SPEED);  
			osDelay(DELAY_STOP);
					
			move(CMD_LEFT45, SD_SPEED);  //45 degree
			osDelay(DELAY_LEFT_TURN);
		
			offRed();
			osDelay(DELAY_STOP);
			
			distance = getDistance();
			while (distance >= DISTANCE_THRESHOLD) {
				onRed();
				distance = getDistance();
				float dist = distance;
				move(CMD_FORWARD, SD_SPEED);  
			}
			offRed();
			move(CMD_STOP, SD_SPEED);  
			osDelay(DELAY_STOP);
			break;
		}
	}
}




/* LED THREADS */
void app_running_front_led(void *argument) {
	for (;;) {
		osEventFlagsWait(movementFlag, 0x0001, osFlagsWaitAny, osWaitForever);
		runningFrontLED();
	}
}

void app_on_front_led(void *argument) {
	for (;;) {
		osEventFlagsWait(movementFlag, 0x0000, osFlagsWaitAny, osWaitForever);
		onAllLED();
	}
}

void app_moving_rear_led(void* argument) {
	for (;;) {
		osEventFlagsWait(movementFlag, 0x0001, NULL, osWaitForever);
		rearLed250();
	}
}

void app_stop_rear_led(void *argument) {
	for (;;) {
		osEventFlagsWait(movementFlag, 0x0000, NULL, osWaitForever);
		rearLed500();
	}
}

/* MOTOR THREAD */
void app_control_motor(void *argument) {
	for (;;) {
		osEventFlagsWait(movementFlag, 0x0001, osFlagsWaitAny, osWaitForever);
		switch (rx_data) {
			case CMD_FORWARD:
				move(CMD_FORWARD, FULL_SPEED); //forwards(FULL_SPEED);
				break;
			case CMD_REVERSE:
				move(CMD_REVERSE, FULL_SPEED); //reverse(FULL_SPEED);
				break;
			case CMD_RIGHT:
				move(CMD_RIGHT, SLOW_SPEED); //right(SLOW_SPEED);
				break;
			case CMD_LEFT:
				move(CMD_LEFT, SLOW_SPEED); //left(SLOW_SPEED);
				break;
			case CMD_LEFT45:
				move(CMD_LEFT45, SLOW_SPEED); //left45(SLOW_SPEED);
				break;
			case CMD_RIGHT90:
				move(CMD_RIGHT90, SLOW_SPEED); //right90(SLOW_SPEED);
				break;
			default:
				stop_moving();
		}
	}
}

/* BUZZER THREAD */
void app_control_buzzer(void *argument) {
	for (;;) {
		osEventFlagsWait(movementFlag, 0x0001, osFlagsWaitAny, osWaitForever);
		osEventFlagsWait(buzzerFlag, 0x0000, osFlagsWaitAny, osWaitForever);
		playSong(); //Play tune while moving defined in the control_threads
	}
}

void app_control_completed_buzzer(void *argument) {
	for (;;) {
		osEventFlagsWait(buzzerFlag, 0x0001, osFlagsWaitAny, osWaitForever);
		playCompletedSong();
	}
}

/* BRAIN THREAD */ 
void app_control_threads(void *argument) {
	for (;;) {
		switch(rx_data) {
			case CMD_FORWARD:
			case CMD_REVERSE:
			case CMD_LEFT:
			case CMD_RIGHT:
			case CMD_LEFT45:
			case CMD_RIGHT90:
			case CMD_SELF_DRIVING:
				osEventFlagsSet(movementFlag, 0x0001);
			case CMD_STOP:
				osEventFlagsSet(movementFlag, 0x0000); //Not sure if this is necessary
				break;
			case CMD_BUZZER:
				osEventFlagsSet(buzzerFlag, 0x0001);
				break;
			default:
				osEventFlagsSet(movementFlag, 0x0000);
				stop_moving();
				break;
		}
	}
}

static void delay(volatile uint32_t nof) {
  while(nof!=0) {
    __asm("NOP");
    nof--;
  }
}

/*
Temporary Test Function for Ultrasonic
*/
void testUltrasonic() {
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
	buzzerFlag = osEventFlagsNew(NULL);
	selfDrivingFlag = osEventFlagsNew(NULL);
	remoteFlag = osEventFlagsNew(NULL);
	movementFlag = osEventFlagsNew(NULL); 
	//stopFlag = osEventFlagsNew(NULL);
	
	//Default to remote mode
	osEventFlagsSet(remoteFlag, 0x0001); 
	
	//Red LED for debugging purposes
	InitRedLED();
	
	rx_data = 0;
	
	osKernelInitialize();
	
	buzzerSem = osSemaphoreNew(1, 1, NULL); //1 available so can start tune immediately
	movementSem = osSemaphoreNew(1, 1, NULL); //Not sure about this implementation
	
	osThreadNew(app_control_threads, NULL, NULL); //Initialize the main thread that controls packets
	osThreadNew(app_running_front_led, NULL, NULL);
	osThreadNew(app_on_front_led, NULL, NULL);
	osThreadNew(app_stop_rear_led, NULL, NULL);
	osThreadNew(app_moving_rear_led, NULL, NULL);
	osThreadNew(app_control_buzzer, NULL, NULL);
	osThreadNew(app_control_motor, NULL, NULL);
	osThreadNew(app_control_completed_buzzer, NULL, NULL);
	osThreadNew(app_self_driving, NULL, &priorityAboveNormal); //should put it to a higher priority actually...
	
	//Init motor, buzzer and led msgs. Using rx_data instead of myDataPkt as we are not using structures.
	/*
	motorMsg = osMessageQueueNew(QUEUE_MSG_COUNT, sizeof(rx_data) , NULL);
	buzzerMsg = osMessageQueueNew(QUEUE_MSG_COUNT, sizeof(rx_data), NULL);
	completedBuzzerMsg = osMessageQueueNew(QUEUE_MSG_COUNT, sizeof(rx_data), NULL);
	frontLedMsg = osMessageQueueNew(QUEUE_MSG_COUNT, sizeof(rx_data), NULL);
	rearLedMsg = osMessageQueueNew(QUEUE_MSG_COUNT, sizeof(rx_data), NULL);
	selfDrivingMsg = osMessageQueueNew(QUEUE_MSG_COUNT, sizeof(rx_data), NULL);
	*/
	
	osKernelStart();
	
	for (;;) { 
		//onAllLED();
		//controlLED(); 
	}
	
}