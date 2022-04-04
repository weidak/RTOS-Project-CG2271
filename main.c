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

osEventFlagsId_t buzzer_flag, self_driving_flag, remote_flag, movement_flag;

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
			case 0x05:
			case 0x06:
				osEventFlagsSet(movement_flag, 0x0001);
				break;
			case 0x07: //set flag for self driving mode
				osEventFlagsSet(self_driving_flag, 0x0001);
				osEventFlagsSet(remote_flag, 0x0000);
				break;
			case 0x08: //set flag for remote mode
				osEventFlagsSet(self_driving_flag, 0x0000);
				osEventFlagsSet(remote_flag, 0x0001);
				break;
			case 0x09:
				osEventFlagsSet(buzzer_flag, 0x0001); //temporary means to switch the buzzer to completed tune
				break;
			default:
				osEventFlagsSet(movement_flag, 0x0000);
				break;
		}
	}
	
	if (UART2->S1 & (UART_S1_OR_MASK | UART_S1_NF_MASK | UART_S1_FE_MASK | UART_S1_PF_MASK)) {
		//handle error
		//clear flag
	}
}

volatile int state = 0;
volatile float distance;

void app_self_driving(void *argument) {
	uint32_t receivedData;
	for (;;) {
		osMessageQueueGet(selfDrivingMsg, &receivedData, NULL, osWaitForever);
		osEventFlagsWait(self_driving_flag, 0x0001, osFlagsWaitAny, osWaitForever);
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
			distance = DISTANCE_THRESHOLD;
			while (distance >= DISTANCE_THRESHOLD) {
					offRed();
					distance = getDistance();
					forwards(SD_SPEED); 
			}
			onRed();
			stop_moving();
			osDelay(DELAY_STOP);
			left45(SD_SPEED); // move(CMD_STOP, SD_SPEED); //
			//osDelay(DELAY_STOP);
			//move(CMD_LEFT45, SD_SPEED);   //45 degree 
			//osDelay(DELAY_LEFT_TURN);

			forwards(SD_SPEED); //move(CMD_FORWARD, SD_SPEED); 
			osDelay(DELAY_STRAIGHT);
			stop_moving(); //move(CMD_STOP, SD_SPEED);   
			osDelay(DELAY_STOP);//osDelay(DELAY_STOP);
			right90(SD_SPEED); //move(CMD_RIGHT90, SD_SPEED); //90 degree
			//osDelay(DELAY_RIGHT_TURN);
			
			forwards(SD_SPEED); //move(CMD_FORWARD, SD_SPEED);  
			osDelay(DELAY_STRAIGHT);
			stop_moving();//move(CMD_STOP, SD_SPEED);  
			osDelay(DELAY_STOP);
			right90(SD_SPEED); //move(CMD_RIGHT90, SD_SPEED); //90 degree
			//osDelay(DELAY_RIGHT_TURN);
			
			forwards(SD_SPEED); //move(CMD_FORWARD, SD_SPEED);  
			osDelay(DELAY_STRAIGHT);
			stop_moving(); //move(CMD_STOP, SD_SPEED);  
			osDelay(DELAY_STOP);//osDelay(DELAY_STOP);
			right90(SD_SPEED); //move(CMD_RIGHT90, SD_SPEED); //90 degree
			//osDelay(DELAY_RIGHT_TURN);
			
			forwards(SD_SPEED); //move(CMD_FORWARD, SD_SPEED);  
			osDelay(DELAY_STRAIGHT);
			stop_moving();//move(CMD_STOP, SD_SPEED);  
			osDelay(DELAY_STOP);
					
			left45(SD_SPEED); // move(CMD_STOP, SD_SPEED); //
			//osDelay(DELAY_STOP);
		
			offRed();
			
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
			
			/*switch (state) {
				case 0: //Object Detection State
					onRed(); //Object not yet detected
					forwards(SD_SPEED);
					if (distance < DISTANCE_THRESHOLD) {
						offRed(); //for visual confirmation that it has been detected
						stop_moving();
						delay(DELAY_STOP);
						state++; 
					} 
					break;
				case 1: //Turn 45 degrees left
					left45(SD_SPEED); 
					delay(DELAY_LEFT_TURN);
					state++;
					break;
				case 2: //Repeat 3 times
				case 3:
				case 4:
					/*forwards(SD_SPEED); 
					delay(DELAY_STRAIGHT);
					stop_moving();
					delay(DELAY_STOP);
					right90(SD_SPEED);
					delay(DELAY_RIGHT_TURN);
					state++;
					offRed();
					stop_moving();
					break;
				case 5: //Last straight
					forwards(SD_SPEED); 
					delay(DELAY_STRAIGHT);
					stop_moving();
					delay(DELAY_STOP);
					state++;
					break;
				case 6: //Turn 45 degrees left
					left45(SD_SPEED);
					delay(DELAY_LEFT_TURN);
					state++;
					break;
				case 7: //State to return to the start. Similar to state 1
					if (distance < DISTANCE_THRESHOLD) {
						//for visual confirmation that it has been detected
						offRed(); 
						stop_moving();
						//state = 0;
						//Exit self-driving mode by itself
						//osEventFlagsSet(self_driving_flag, 0x0000);
						//osEventFlagsSet(remote_flag, 0x0001);
					} else {
						//Object not yet detected
						onRed(); 
						forwards(SD_SPEED); 
					}
					state++;
					break;
				default: 
					//If somehow it reaches here, it forces it back to initial state
					delay(0x80000);
					state = 0;
			}
			//osDelay(20);
		}*/


		/*
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
			case 1: //Turn 45 degrees left
				left45(SD_SPEED); 
				osDelay(DELAY_LEFT_TURN);
				state++;
				break;
			case 2: //Repeat 3 times
			case 3:
			case 4:
				forwards(SD_SPEED); 
				osDelay(DELAY_STRAIGHT);
				stop_moving();
				osDelay(DELAY_STOP);
				right90(SD_SPEED);
				osDelay(DELAY_RIGHT_TURN);
				state++;
				break;
			case 5: //Last straight
				forwards(SD_SPEED); 
				osDelay(DELAY_STRAIGHT);
				stop_moving();
				osDelay(DELAY_STOP);
				state++;
				break;
			case 6: //Turn 45 degrees left
				left45(SD_SPEED);
				osDelay(DELAY_LEFT_TURN);
				state++;
			case 7: //State to return to the start. Similar to state 1
				if (distance < DISTANCE_THRESHOLD) {
					//for visual confirmation that it has been detected
					offRed(); 
					stop_moving();
					state = 0;
					//Exit self-driving mode by itself
					osEventFlagsSet(self_driving_flag, 0x0000);
					osEventFlagsSet(remote_flag, 0x0001);
				} else {
					//Object not yet detected
					onRed(); 
					forwards(SD_SPEED); 
				}
			default: 
				//If somehow it reaches here, it forces it back to initial state
				state = 0;
				break;
		}
		*/
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
			case 0x05:
			case 0x06:
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
		//osEventFlagsWait(movement_flag, 0x0001, osFlagsWaitAny, osWaitForever);
		osMessageQueueGet(motorMsg, &receivedData, NULL, osWaitForever);
		switch (receivedData) {
			case 0x01:
				left45(FULL_SPEED); //forwards(FULL_SPEED); // 
				break;
			case 0x02:
				right90(FULL_SPEED); //;reverse(FULL_SPEED); // 
				break;
			case 0x03:
				right(SLOW_SPEED);
				break;
			case 0x04:
				left(SLOW_SPEED);
				break;
			case 0x05:
				left45(SLOW_SPEED);
				osDelay(DELAY_LEFT_TURN);
				break;
			case 0x06:
				right90(SLOW_SPEED);
				osDelay(DELAY_RIGHT_TURN);
				break;
			case 0x07: //self driving mode
				//do nothing, should not stop moving
				break;
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
			case 0x09: // When the robot finishes moving
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
		//osMessageQueuePut(completedBuzzerMsg, &currCmd, 1U, 0); //should compete for semaphore and buzzerMsg will not be able to obtain semaphore
		osMessageQueuePut(buzzerMsg, &currCmd, 0U, 0);
		osMessageQueuePut(motorMsg, &currCmd, 1U, 0); //To update with priorities
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
	movement_flag = osEventFlagsNew(NULL); 
	
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
	movementSem = osSemaphoreNew(1, 1, NULL);
	
	osThreadNew(control_threads, NULL, NULL); //Initialize the main thread that controls packets
	osThreadNew(app_control_front_led, NULL, NULL);
	osThreadNew(app_control_rear_led, NULL, NULL);
	osThreadNew(app_control_buzzer, NULL, NULL);
	osThreadNew(app_control_motor, NULL, NULL);
	osThreadNew(app_control_completed_buzzer, NULL, NULL);
	osThreadNew(app_self_driving, NULL, &priorityAboveNormal); //should put it to a higher priority actually...
	
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