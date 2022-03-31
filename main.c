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
osMessageQueueId_t motorMsg, buzzerMsg, frontLedMsg, rearLedMsg, ledMsg;

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
		if ((rx_data == 0x01 || rx_data == 0x02 || rx_data == 0x03 || rx_data == 0x04)) {
			assert(rx_data != 00);
			runningFrontLED(&receivedData);
		}
		else {
			onAllLED();
		}
		/*
		switch (receivedData) {
			case 0x01:
			case 0x02:
			case 0x03:
			case 0x04:
				//Individually light up the led while moving
				runningFrontLED(receivedData);
				break;
			default:
				//On front lights while stationary
				onAllLED();
				break;
		}*/
	}
}
/*
void app_control_led(void *argument) {
	uint32_t receivedData;
	for (;;) {
		osMessageQueueGet(ledMsg, &receivedData, NULL, osWaitForever);
		switch (receivedData) {
			case 0x01:
			case 0x02:
			case 0x03:
			case 0x04:
				//Individually light up the led while moving
				runningFrontLED();
				break;
			default:
				//On front lights while stationary
				onAllLED();
				break;
		}
	}
}*/

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
			default:
				break; //Do nothing if not moving
		}
		
	}
}

void control_threads(void *argument) {
	//Not sure if its supposed to be in a forever while loop?
	//rx_data will never change again until another command is sent,
	//Hence I set the currCmd back to 0 again at the end after a configurable delay
	uint32_t currCmd;
	while (1) {
		currCmd = UART2->S1 & UART_S1_RDRF_MASK ? 0x00 : rx_data; 
		osMessageQueuePut(motorMsg, &currCmd, NULL, 0); //To update with priorities
		osMessageQueuePut(frontLedMsg, &currCmd, NULL, 0);
		osMessageQueuePut(rearLedMsg, &currCmd, NULL, 0);
		osMessageQueuePut(buzzerMsg, &currCmd, NULL, 0);
		//currCmd = 0x00; //Default state
	}

	//osDelay(500); //delay for half a second, just for buffer and testing 
	/*
	//For future streamlining purposes, yet to find a use
	switch (currCmd) {
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		default:
			
	}
	*/
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

/*******************
	ULTRASONIC SENSOR
********************/
#define TRIGGER_PIN 30
#define ECHO_PIN 31
#define AIR_SPEED 34000

void InitUltra() {
	SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;
	SIM_SCGC6 |= SIM_SCGC6_TPM0_MASK | SIM_SCGC6_PIT_MASK;
	SIM->SOPT2 &= ~SIM_SOPT2_TPMSRC_MASK;	
	SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1); 	
	//Configure PIT for 10 us pulse generation
	PIT->MCR &= ~PIT_MCR_MDIS_MASK;
	PIT->CHANNEL[0].TCTRL |= PIT_TCTRL_TIE_MASK | PIT_TCTRL_TEN_MASK;
	PIT->CHANNEL[0].LDVAL = 2000;//for 10us pulse
	//Configure TPM0 to input capture mode
	TPM0->SC &= ~((TPM_SC_CMOD_MASK) | (TPM_SC_PS_MASK) | TPM_SC_CPWMS_MASK);
	TPM0->SC |= TPM_SC_CMOD(1) | TPM_SC_PS(7);
	TPM0->MOD = 3750000; //10 s timer, will reset every 1 s
	TPM0_C4SC |= TPM_CnSC_CHIE_MASK | TPM_CnSC_ELSA(1) | TPM_CnSC_ELSB(1);
	//GPIO configuration
	PORTE->PCR[TRIGGER_PIN] &= ~PORT_PCR_MUX_MASK;
	PORTE->PCR[TRIGGER_PIN] |= PORT_PCR_MUX(1);
	PORTE->PCR[ECHO_PIN] &= ~PORT_PCR_MUX_MASK;
	PORTE->PCR[ECHO_PIN] |= PORT_PCR_MUX(3);
	//Set GPIO direction
	PTE->PDDR |= MASK(TRIGGER_PIN);
	PTE->PDDR &= ~MASK(ECHO_PIN);
}

volatile int firstTime = 0;
volatile int flagTRIGGER, flagRising, flagFalling = 0;

void PIT_IRQHandler() { 
	//Clear Pending IRQ
	NVIC_ClearPendingIRQ(PIT_IRQn);
	//Configure output pin
	if (firstTime == 0) {
		flagTRIGGER = 1;
		PTD->PDOR |= MASK(TRIGGER_PIN);
	}
	else if (firstTime == 1) {
		PTD->PDOR &= ~MASK(TRIGGER_PIN);
		PIT->MCR &= ~PIT_TCTRL_TEN_MASK;
		//NVIC_DisableIRQ(PIT_IRQn);
	}
	firstTime = ~firstTime;
	//Clear Flag
	PIT->CHANNEL[0].TFLG &= PIT_TFLG_TIF_MASK;
}

volatile int counter = 0;
volatile uint32_t start_time = 0;
volatile uint32_t stop_time = 0;

void TPM0_IRQHandler() {
	if (counter % 2 == 0) {
		//rising edge
		flagRising = 1;
		start_time = TPM0_C4V;
	}
	else {
		flagFalling = 1;
		stop_time = TPM0_C4V;
	}
	counter = ~counter;
	//Clear Flag
	TPM0_C4SC |= TPM_CnSC_CHF_MASK;
}

float getDistance() {
	PTE->PDOR |= MASK(TRIGGER_PIN);
	delay(0x100);
	PTE->PDOR &= ~MASK(TRIGGER_PIN);
	delay(0x3333);//ard 30 ms delay to read start and stop time
	uint32_t start = start_time;
	uint32_t stop = stop_time;
	float time = ((float)(stop_time - start_time)) / 375000.0;
	float distance = AIR_SPEED * time / 2;
	int i = 0;
	return distance;

}

int main() {
	SystemCoreClockUpdate();
	initGPIOLed();
	
	InitUART2(BAUD_RATE);
	InitPWMMotors();
	//InitPWMBuzzer();
	
	//InitGPIOBuzzer();
	//InitGPIOMotors();
	
	InitUltra();
	
	SIM_SCGC5 |= SIM_SCGC5_PORTB_MASK;
	PORTB->PCR[RED_LED] &= ~PORT_PCR_MUX_MASK;
	PORTB->PCR[RED_LED] |= PORT_PCR_MUX(1);
	PTB->PDDR |= MASK(RED_LED);
	
	rx_data = 0;
	//forwards(SLOW_SPEED);

	while (1) {
		//Reset counter
		//start_time = 0;
		//stop_time = 0;
		int flagReset = flagTRIGGER;
		TPM0_SC &= ~TPM_SC_CMOD_MASK;
		TPM0->SC |= TPM_SC_CMOD(1);
		TPM0_C4SC |= TPM_CnSC_CHIE_MASK | TPM_CnSC_ELSA(1) | TPM_CnSC_ELSB(1);
		TPM0_CNT = 0;
		counter = 0;
		firstTime = 0;
		PIT->CHANNEL[0].LDVAL = 104;
		//Enable interrupts for PIT and TPM0
		NVIC_EnableIRQ(TPM0_IRQn);
		NVIC_EnableIRQ(PIT_IRQn);
		float distance = getDistance();
		if (distance < 20) {
			onRed();
		} 
		else{
			offRed();
		}
		int flag = flagTRIGGER;
		flagTRIGGER = 0;
		int i = 0;
	}
	/*
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
	*/
	/*
	osKernelInitialize();
	
	osThreadNew(control_threads, NULL, NULL); //Initialize the main thread that controls packets
	osThreadNew(app_control_front_led, NULL, NULL);
	osThreadNew(app_control_rear_led, NULL, NULL);
	//osThreadNew(app_control_led, NULL, NULL);
	osThreadNew(app_control_buzzer, NULL, NULL);
	osThreadNew(app_control_motor, NULL, NULL);
	
	//Init motor, buzzer and led msgs. Using rx_data instead of myDataPkt as we are not using structures.
	motorMsg = osMessageQueueNew(QUEUE_MSG_COUNT, sizeof(rx_data) , NULL);
	buzzerMsg = osMessageQueueNew(QUEUE_MSG_COUNT, sizeof(rx_data), NULL);
	//ledMsg = osMessageQueueNew(QUEUE_MSG_COUNT, sizeof(rx_data), NULL);
	frontLedMsg = osMessageQueueNew(QUEUE_MSG_COUNT, sizeof(rx_data), NULL);
	rearLedMsg = osMessageQueueNew(QUEUE_MSG_COUNT, sizeof(rx_data), NULL);


	osKernelStart();
	
	for (;;) { 
		//onAllLED();
		//controlLED(); 
	}
	*/
}