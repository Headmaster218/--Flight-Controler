#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "sys.h" 
 	
extern u16 USART_RX_STA;         		//����״̬���	

void UART_GPS_Init(u32 bound);
void UART_24G_Init(u32 bound);
#endif


