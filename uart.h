#include "MKL25Z4.h"                    // Device header

#ifndef UART
#define UART

#define MASK(x) (1<<(x))

#define BAUD_RATE 9600
#define UART_TX_PORTE22 22 // Page 162 of datasheet
#define UART_RX_PORTE23 23
#define UART2_INT_PRIO 128

void InitUART2(uint32_t baud_rate);

void UART2_IRQHandler();


#endif