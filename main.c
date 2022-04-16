#include "RTE_Components.h"
#include "cmsis_os2.h"
#include "MKL25Z4.h"                    // Device header
#include "buzzerControls.h"
#include "motorControls.h"
#include "uart.h"
#include "ledControls.h"
#include "ultrasonic.h"

#define QUEUE_MSG_COUNT 1

#define CMD_FORWARD 0x01
#define CMD_REVERSE 0x02
#define CMD_RIGHT 0x03
#define CMD_LEFT 0x04
#define CMD_LEFT_STATIONARY 0x05
#define CMD_RIGHT_STATIONARY 0x06
#define CMD_SELF_DRIVING 0x07
#define CMD_STOP 0x08
#define CMD_BUZZER 0x09

//Initialize the priorities
const osThreadAttr_t priorityAboveNormal = {
  .priority = osPriorityAboveNormal
};

const osThreadAttr_t priorityAboveNormal1 = {
	.priority = osPriorityAboveNormal1
};

const osThreadAttr_t priorityAboveNormal2 = {
	.priority = osPriorityAboveNormal2
};

volatile float distance = 0;
volatile int state = 1;
volatile uint32_t buzzerStatus = 0x00;
volatile uint32_t rx_data;

osMessageQueueId_t motorMsg, buzzerMsg, frontLedMsg, rearLedMsg;

osSemaphoreId_t buzzerSem;

osThreadId_t selfDrivingId;

/*---------------------
FOR DEBUGGING PURPOSES
----------------------*/
#define RED_LED 18
#define GREEN_LED 19

void onRed() {
	PTB->PDOR &= ~MASK(RED_LED);
}

void onGreen() {
	PTB->PDOR &= ~MASK(GREEN_LED);
}

void offRed() {
	PTB->PDOR |= MASK(RED_LED);
}

void offGreen() {
	PTB->PDOR |= MASK(GREEN_LED);
}

void InitRedGreenLED() {
	SIM_SCGC5 |= SIM_SCGC5_PORTB_MASK;
	PORTB->PCR[RED_LED] &= ~PORT_PCR_MUX_MASK;
	PORTB->PCR[RED_LED] |= PORT_PCR_MUX(1);
	PORTB->PCR[GREEN_LED] &= ~PORT_PCR_MUX_MASK;
	PORTB->PCR[GREEN_LED] |= PORT_PCR_MUX(1);
	PTB->PDDR |= MASK(RED_LED);
	PTB->PDDR |= MASK(GREEN_LED);
}

/* ------------------------------------
	UART2 IRQ Handler
------------------------------------ */

void UART2_IRQHandler() {
	
	NVIC_ClearPendingIRQ(UART2_IRQn);
	
	uint32_t currCmd = 0;
	
	if (UART2->S1 & UART_S1_RDRF_MASK) {
		rx_data = UART2->D;
		if (rx_data == CMD_SELF_DRIVING) osThreadFlagsSet(selfDrivingId, 0x0001);
	}
	
	if (UART2->S1 & (UART_S1_OR_MASK | UART_S1_NF_MASK | UART_S1_FE_MASK | UART_S1_PF_MASK)) {
		//Handle error and clear flag
	}
}

/* ------------------------------------
	Self-Driving Thread
------------------------------------ */

void InitSelfDriving() {
		TPM2_SC &= ~TPM_SC_CMOD_MASK;
		TPM2_C0SC |= TPM_CnSC_CHIE_MASK | TPM_CnSC_ELSA(1) | TPM_CnSC_ELSB(1);
		TPM2_CNT = 0;
		NVIC_EnableIRQ(TPM2_IRQn);
		TPM2->SC |= TPM_SC_CMOD(1);
}

int counter_ultra = 0;

void tSelfDriving(void *argument) {
	for (;;) {
		osThreadFlagsWait(0x0001, osFlagsWaitAny, osWaitForever);
		osSemaphoreRelease(buzzerSem);
		osThreadFlagsWait(0x0001, osFlagsWaitAny, osWaitForever);
		osSemaphoreRelease(buzzerSem);
		uint32_t rx = rx_data;
		int fast_stop = 0;
		InitSelfDriving();
		while (1) {			
			distance = getDistance();
			switch (state) {
				case 1:
					offRed();
					if (counter_ultra == 0) {
						forwards(FIRST_FORWARD_SPEED);
					}
					else{
						forwards(3000);
					}
					if (distance < DISTANCE_THRESHOLD) {
						if (counter_ultra == 1) {
							state++;
							onRed();
							counter_ultra = 0;
						}
						
						counter_ultra++;
					}
					break;
				case 2:
					stop_moving();
					osDelay(DELAY_STOP);
					reverse(SD_SPEED);
					osDelay(DELAY_REV);
					stop_moving();
					osDelay(DELAY_STOP);
					left45(SD_SPEED); 
					state++;
					break;
				case 3:
				case 4:
				case 5:
					forwards(SD_SPEED);   
					osDelay(DELAY_STRAIGHT);
					stop_moving();
					osDelay(DELAY_STOP);
					right90(SD_SPEED); 
					state++;
					break;
				case 6:
					forwards(SD_SPEED);
					osDelay(DELAY_STRAIGHT + 150);
					stop_moving();
					osDelay(DELAY_STOP);
					left45(SD_SPEED - 50); 
					state++;
					break;
				case 7:
					offRed();
					forwards(RETURN_FORWARD_SPEED);
					if (distance < DISTANCE_THRESHOLD - 10) {
						if (counter_ultra == 1) {
							state++;
							onRed();
							counter_ultra = 0;
						}
						counter_ultra++;
					}
					break;
				default:
					if (fast_stop == 0) {
						reverse(SD_SPEED);
						osDelay(50);
						fast_stop = 1;
					}
					stop_moving();
					osDelay(DELAY_STOP);
					rx_data = 0x00; //Force rx_data to change back to 0
					buzzerStatus = 1;
			}
			
			int state_case = state;
			float dist = distance;
		}
	}
}

/* ------------------------------------
	LED Control Functions and Threads
------------------------------------ */

uint32_t checkMove(uint32_t cmd) {
	switch (cmd) {
		case CMD_FORWARD:
		case CMD_REVERSE:
		case CMD_LEFT:
		case CMD_RIGHT:
		case CMD_LEFT_STATIONARY:
		case CMD_RIGHT_STATIONARY:
		case CMD_SELF_DRIVING:
			return 0x01;
			break;
		default:
			return 0x00;
	}
}

void tRearLed(void* argument) {
	uint32_t receivedData;
	for (;;) {
		osMessageQueueGet(frontLedMsg, &receivedData, NULL, osWaitForever);
		if (receivedData == 0x01) {
			rearLed500();
		} else {
			rearLed250();
		}
	}
}

void tFrontLed(void *argument) {
	uint32_t receivedData;
	for (;;) {
		osMessageQueueGet(frontLedMsg, &receivedData, NULL, osWaitForever);
		if (receivedData == 0x01) {
			runningFrontLED();
		} else {
			onAllLED();
		}
	}
}

/* ------------------------------------
	Motor Control Thread
------------------------------------ */

void tMotor(void *argument) {
	uint32_t currCmd;
	for (;;) {
		currCmd = rx_data;
		switch (currCmd) {
			case CMD_FORWARD:
				forwards(FULL_SPEED); 
				osSemaphoreRelease(buzzerSem);
				break;
			case CMD_REVERSE:
				reverse(FULL_SPEED); 
				break;
			case CMD_RIGHT:
				right(HALF_SPEED);
				break;
			case CMD_LEFT:
				left(HALF_SPEED);
				break;
			case CMD_LEFT_STATIONARY:
				left_stationary(HALF_SPEED);
				break;
			case CMD_RIGHT_STATIONARY:
				right_stationary(HALF_SPEED);
				break;
			case CMD_SELF_DRIVING: //self driving mode
				//do nothing, should not stop moving
				break;
			default:
				stop_moving();
				offGreen();
				break;
		}
	}
}
	
/* ------------------------------------
	Buzzer-related Functions and Threads
------------------------------------ */

char coffins[] = "g gDC l a aaC lag gLALALg gLALALg gDC l a aaC lag gLALALg gLALALllllDDDDCCCCFFFFGGGGGGGGGGGGClaf";
int coffinBeats[] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
int coffinSongLength = 95;
int coffinTempo = 150;

int coffin_frequency(char note) {
	char names[] = {'f', 'g', 'a', 'l', 'C', 'D', 'E', 'F', 'G', 'A', 'L'};
	int frequencies[] = {175, 196, 220, 233, 262, 294, 330, 349, 392, 440, 466};
		for (int i = 0; i < sizeof(frequencies)/sizeof(frequencies[0]); i++) {
			if (names[i] == note) return (frequencies[i]);
		}
		return 0;
}

void playCoffin() {
		buzzerStatus = 0;
		for (int i = 0; i <= coffinSongLength; i++) {
			osSemaphoreAcquire(buzzerSem, osWaitForever);
			TPM1->MOD = FREQ2MOD(coffin_frequency(coffins[i])); 
			TPM1_C0V = (FREQ2MOD(coffin_frequency(coffins[i])))/5; 
			osSemaphoreRelease(buzzerSem);
			osDelay(coffinBeats[i] * coffinTempo);		
			if (i == coffinSongLength) {
				i = 0;
			} 
			if (buzzerStatus == 1) {
				playSong();
				break;
			}
		}
		osSemaphoreRelease(buzzerSem);
}

void tBuzzer(void *argument) {
	uint32_t receivedData;
	for (;;) {
		osMessageQueueGet(buzzerMsg, &receivedData, NULL, osWaitForever);
		if (receivedData == 0x00) {
			playCoffin();
		}
	}
}

void checkBuzzer(uint32_t cmd) {
	if (cmd == CMD_BUZZER) {
		buzzerStatus = 0x01;
	}
}

/* ------------------------------------
			Brain Thread
------------------------------------ */

void tBrain(void *argument) {
	uint32_t currCmd;
	uint32_t ledStatus;
	while (1) {
		currCmd = rx_data;
		ledStatus = checkMove(currCmd);
		checkBuzzer(currCmd);
		osMessageQueuePut(buzzerMsg, &currCmd, 0U, 0);
		osMessageQueuePut(motorMsg, &currCmd, 1U, 0); 
		osMessageQueuePut(frontLedMsg, &ledStatus, 0U, 0);
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
	
	//Red and Green LED for debugging purposes
	InitRedGreenLED();
	
	rx_data = 0;

	osKernelInitialize();
	
	buzzerSem = osSemaphoreNew(1, 0, NULL); // Initialize only when moving, thus max 1 initial 0
	
	//Self Driving thread
	selfDrivingId = osThreadNew(tSelfDriving, NULL, NULL); 
	
	osThreadNew(tBrain, NULL, NULL); 
	osThreadNew(tFrontLed, NULL, NULL);
	osThreadNew(tRearLed, NULL, NULL);
	osThreadNew(tBuzzer, NULL, NULL);
	osThreadNew(tMotor, NULL, NULL);

	motorMsg = osMessageQueueNew(QUEUE_MSG_COUNT, sizeof(rx_data) , NULL);
	buzzerMsg = osMessageQueueNew(QUEUE_MSG_COUNT, sizeof(rx_data), NULL);
	frontLedMsg = osMessageQueueNew(QUEUE_MSG_COUNT, sizeof(rx_data), NULL);
	rearLedMsg = osMessageQueueNew(QUEUE_MSG_COUNT, sizeof(rx_data), NULL);

	osKernelStart();
	
	for (;;) { }
	
}