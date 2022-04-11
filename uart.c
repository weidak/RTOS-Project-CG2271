#include "MKL25Z4.h"                    // Device header
#include "uart.h"

void InitUART2(uint32_t baud_rate) {
	
	uint32_t divisor, bus_clock;
	//Enable clock to UART2 and PORTE
	SIM->SCGC4 |= SIM_SCGC4_UART2_MASK;
	SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK;
	
	//Connect UART pins for PTE22, PTE23
	PORTE->PCR[UART_TX_PORTE22] &= ~PORT_PCR_MUX_MASK; //clear the configuration
	PORTE->PCR[UART_TX_PORTE22] |= PORT_PCR_MUX(4); //set to 4 for uart
	
	PORTE->PCR[UART_RX_PORTE23] &= ~PORT_PCR_MUX_MASK; //clear mux
	PORTE->PCR[UART_RX_PORTE23] |= PORT_PCR_MUX(4); //set to 4 for uart
	
	//Disable tx and rx before configuration
	UART2->C2 &= ~((UART_C2_TE_MASK) | (UART_C2_RE_MASK));
	
	bus_clock = (DEFAULT_SYSTEM_CLOCK)/2; //halved to 24MHz due to how the module is designed
	divisor = bus_clock / (baud_rate * 16);
	UART2->BDH = UART_BDH_SBR(divisor >> 8);
	UART2->BDL = UART_BDL_SBR(divisor);
	
	//No parity, 8-bit transmission
	UART2->C1 = 0; //no parity as we dont use
	UART2->S2 = 0; //error-related flags
	UART2->C3 = 0; //enable interrupts for error checks
	
	//UART2->C2 |= ((UART_C2_TE_MASK) | (UART_C2_RE_MASK)); //transmit enable and receive enable
	
	UART2->C2 |= UART_C2_RE_MASK; //receiver only as we only need to process receiveing the data
	
	//Initialize peripheral
	NVIC_SetPriority(UART2_IRQn, UART2_INT_PRIO);
	NVIC_ClearPendingIRQ(UART2_IRQn);
	NVIC_EnableIRQ(UART2_IRQn);
		
	UART2->C2 |= UART_C2_RIE_MASK;
	
}

