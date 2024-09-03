#ifndef __usart2_H
#define __usart2_H	 
#include "stm32f10x.h"
#include "stm32f10x_usart.h"

#define USART2_RXBUFF_SIZE   48 

extern unsigned int  Rx2Counter;          //�ⲿ�����������ļ����Ե��øñ���
extern unsigned char Usart2RecBuf[USART2_RXBUFF_SIZE]; //�ⲿ�����������ļ����Ե��øñ���

void USART2_Init(u32 baud);
void USART2_Sned_Char(u8 temp);
void Uart2_SendStr(char*SendBuf);
void uart2_send(unsigned char *bufs,unsigned char len);

#endif


