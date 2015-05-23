#ifndef __USART_H__
#define __USART_H__

#include <stm32f10x.h>
#include <stm32f10x_usart.h>
#include <stdio.h>


void USB2Serial_Init(void);
void USART1_Puts(unsigned char * str);
uint8_t USART_Scanf(uint32_t value);


#endif


