#ifndef __USART3_H
#define __USART3_H
#include "stdio.h"	
#include "sys.h"  
//////////////////////////////////////////////////////////////////////////////////	 
									  
////////////////////////////////////////////////////////////////////////////////// 	   

#define USART3_RX_LEN		600					//�����ջ����ֽ���
#define USART3_TX_LEN		600					//����ͻ����ֽ���
#define USART3_RX_EN 			1					//0,������;1,����.

extern u8  USART3_RX_BUF[USART3_RX_LEN]; 		//���ջ���,���USART3_MAX_RECV_LEN�ֽ�
extern u8  USART3_TX_BUF[USART3_TX_LEN]; 		//���ͻ���,���USART3_MAX_SEND_LEN�ֽ�
extern vu16 USART3_RX_STA;   				//��������״̬
extern u8 USART3_RECEIVE_OK;
void usart3_init(u32 bound);				//����2��ʼ�� 
void u3_printf(char* fmt,...);
void u3_printf_json(char* fmt);
#endif













