#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "sys.h" 

#define USART1_RX_LEN  			200  	//�����������ֽ��� 200
#define EN_USART1_RX 			1		//ʹ�ܣ�1��/��ֹ��0������1����
	  	
extern u8  USART1_RX_BUF[USART1_RX_LEN]; //���ջ���,���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з� 
extern u8 USART1_RX_STA;         		//����״̬���	
extern u8 USART1_RECEIVE_OK;

void uart_init(u32 bound);
void send_com(u8 data);
void USART_Send(uint8_t *Buffer, uint8_t Length);
void USART1_send_byte(uint8_t byte);
#endif


