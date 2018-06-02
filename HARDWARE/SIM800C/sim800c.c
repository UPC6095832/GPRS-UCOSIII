#include "sim800c.h"
#include "usart.h"
#include "delay.h"	 	 	 	 	 	 
#include "string.h"    		
#include "usart3.h" 
#include "timer.h"
#include <stdlib.h>//����atoi������ͷ�ļ�
#include "led.h"
#include "includes.h"	//ucos ʹ��	

u8* sim800c_check_cmd(u8 *str)
{
	char *strx=0;
	if(USART3_RECEIVE_OK)		//���յ�һ��������
	{ 
		USART3_RX_BUF[USART3_RX_STA&0X7FFF]=0;//��ӽ�����
		strx=strstr((const char*)USART3_RX_BUF,(const char*)str);
	} 
	return (u8*)strx;
}

u8 sim800c_send_cmd(u8 *cmd,u8 *ack,u16 waittime)
{
	u8 res=0; 
	USART3_RX_STA=0;
	if((u32)cmd<=0XFF)
	{
		while((USART3->SR&0X40)==0);//�ȴ���һ�����ݷ������  
		USART3->DR=(u32)cmd;
	}else u3_printf("%s\r\n",cmd);//��������
	
	if(ack&&waittime)		//��Ҫ�ȴ�Ӧ��
	{
		while(--waittime)	//�ȴ�����ʱ
		{ 

			delay_ms(10);
			if(USART3_RECEIVE_OK)//���յ��ڴ���Ӧ����
			{
				if(sim800c_check_cmd(ack))break;//�õ���Ч���� 
				USART3_RX_STA=0;
                USART3_RECEIVE_OK=0;
			} 
		}
		if(waittime==0)res=1; 
	}
	return res;
} 


u8 gprs_join_server(void)
{	
    if(sim800c_send_cmd("AT+CIPCLOSE=1","CLOSE OK",100)==0)LED2=1;	//�ر�����
	if(sim800c_send_cmd("AT+CIPSHUT","SHUT OK",100)==0)LED3=1;		//�ر��ƶ����� 
	if(sim800c_send_cmd("AT+CGCLASS=\"B\"","OK",1000)==0)LED4=1;				//����GPRS�ƶ�̨���ΪB,֧�ְ����������ݽ��� 
	if(sim800c_send_cmd("AT+CGDCONT=1,\"IP\",\"CMNET\"","OK",100)==0)LED5=1;//����PDP������,��������Э��,��������Ϣ
	if(sim800c_send_cmd("AT+CGATT=1","OK",500)==0)LED6=1;					//����GPRSҵ��
	if(sim800c_send_cmd("AT+CIPCSGP=1,\"CMNET\"","OK",500)==0)LED7=1;	 	//����ΪGPRS����ģʽ
	if(sim800c_send_cmd("AT+CIPHEAD=1","OK",500)==0)LED7=0;	 				//���ý���������ʾIPͷ(�����ж�������Դ)
	if(sim800c_send_cmd("AT+CIPSTART=\"TCP\",\"120.76.210.158\",5002\r\n","OK",500)) u3_printf(" TCP���Ӵ���");		//��������
	else {u3_printf(" TCP����ok");  LED0=1;};	
	return 0;
} 

void _ttywrch(int ch){};











