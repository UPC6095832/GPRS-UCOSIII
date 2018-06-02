#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "sys.h" 

#define USART1_RX_LEN  			200  	//定义最大接收字节数 200
#define EN_USART1_RX 			1		//使能（1）/禁止（0）串口1接收
	  	
extern u8  USART1_RX_BUF[USART1_RX_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern u8 USART1_RX_STA;         		//接收状态标记	
extern u8 USART1_RECEIVE_OK;

void uart_init(u32 bound);
void send_com(u8 data);
void USART_Send(uint8_t *Buffer, uint8_t Length);
void USART1_send_byte(uint8_t byte);
#endif


