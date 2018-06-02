#include "sim800c.h"
#include "usart.h"
#include "delay.h"	 	 	 	 	 	 
#include "string.h"    		
#include "usart3.h" 
#include "timer.h"
#include <stdlib.h>//调用atoi函数的头文件
#include "led.h"
#include "includes.h"	//ucos 使用	

u8* sim800c_check_cmd(u8 *str)
{
	char *strx=0;
	if(USART3_RECEIVE_OK)		//接收到一次数据了
	{ 
		USART3_RX_BUF[USART3_RX_STA&0X7FFF]=0;//添加结束符
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
		while((USART3->SR&0X40)==0);//等待上一次数据发送完成  
		USART3->DR=(u32)cmd;
	}else u3_printf("%s\r\n",cmd);//发送命令
	
	if(ack&&waittime)		//需要等待应答
	{
		while(--waittime)	//等待倒计时
		{ 

			delay_ms(10);
			if(USART3_RECEIVE_OK)//接收到期待的应答结果
			{
				if(sim800c_check_cmd(ack))break;//得到有效数据 
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
    if(sim800c_send_cmd("AT+CIPCLOSE=1","CLOSE OK",100)==0)LED2=1;	//关闭连接
	if(sim800c_send_cmd("AT+CIPSHUT","SHUT OK",100)==0)LED3=1;		//关闭移动场景 
	if(sim800c_send_cmd("AT+CGCLASS=\"B\"","OK",1000)==0)LED4=1;				//设置GPRS移动台类别为B,支持包交换和数据交换 
	if(sim800c_send_cmd("AT+CGDCONT=1,\"IP\",\"CMNET\"","OK",100)==0)LED5=1;//设置PDP上下文,互联网接协议,接入点等信息
	if(sim800c_send_cmd("AT+CGATT=1","OK",500)==0)LED6=1;					//附着GPRS业务
	if(sim800c_send_cmd("AT+CIPCSGP=1,\"CMNET\"","OK",500)==0)LED7=1;	 	//设置为GPRS连接模式
	if(sim800c_send_cmd("AT+CIPHEAD=1","OK",500)==0)LED7=0;	 				//设置接收数据显示IP头(方便判断数据来源)
	if(sim800c_send_cmd("AT+CIPSTART=\"TCP\",\"120.76.210.158\",5002\r\n","OK",500)) u3_printf(" TCP连接错误");		//发起连接
	else {u3_printf(" TCP连接ok");  LED0=1;};	
	return 0;
} 

void _ttywrch(int ch){};











