#include "sys.h"
#include "usart.h"	
#include "string.h"
#include "mytask.h"
#include "includes.h"	//ucos 使用	
	  

/**************加入以下代码,支持printf函数***************/	  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
_sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
    USART1->DR = (u8) ch;      
	return ch;
}
#endif 

#if EN_USART1_RX   //如果使能了接收
 	
u8 USART1_RX_BUF[USART1_RX_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
u8 USART1_RX_STA=0;       //接收状态标记	 
u8 USART1_RECEIVE_OK=0;  

void uart_init(u32 bound){
  //GPIO端口设置
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
	 
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//使能USART1，GPIOA时钟
  
	//USART1_TX   GPIOA.9
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.9
   
  //USART1_RX	  GPIOA.10初始化
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.10  

  //Usart1 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
  
   //USART 初始化设置

	USART_InitStructure.USART_BaudRate = bound;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

  USART_Init(USART1, &USART_InitStructure); //初始化串口1
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启串口接受中断
  USART_Cmd(USART1, ENABLE);                    //使能串口1 

}

void USART1_IRQHandler(void)                	//串口1中断服务程序
	{
    OS_ERR err;   
	OSIntEnter();    

	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)//判断接收标志
	{
		USART1_RX_BUF[USART1_RX_STA++]=USART_ReceiveData(USART1);//读取串口数据，同时清接收标志
		if (USART1_RX_BUF[0]!=0x5a)//帧头不对
			USART1_RX_STA=0;	
		if ((USART1_RX_STA==2)&&(USART1_RX_BUF[1]!=0x5a))//帧头不对
			USART1_RX_STA=0;
	
		if(USART1_RX_STA>3)//i等于4时，已经接收到数据量字节rebuf[3]
		{
			if(USART1_RX_STA==(USART1_RX_BUF[3]+5))//判断是否接收一帧数据完毕
                
            {

			switch(USART1_RX_BUF[2])//接收完毕后处理
			{
				case 0x45:
					if(!USART1_RECEIVE_OK)//当数据处理完成后才接收新的数据
					{
                      
						USART1_RECEIVE_OK=1;//接收完成标志
					}
					break;
				case 0x15:
                    if(!USART1_RECEIVE_OK)//当数据处理完成后才接收新的数据
					{
                       
						USART1_RECEIVE_OK=1;//接收完成标志
					}
                    break;//原始数据接收，可模仿0x45的方式
				case 0x35:break;
			}
              USART1_RX_STA=0;//缓存清0  
            }
			
		}
	}
        

	OSIntExit();  //退出中断，发生一次任务调度											 

} 
    
void send_com(u8 data)
{
	u8 bytes[3]={0};
	bytes[0]=0xa5;
	bytes[1]=data;//功能字节
	USART_Send(bytes,3);//发送帧头、功能字节、校验和
}
void USART_Send(uint8_t *Buffer, uint8_t Length)
{
	uint8_t i=0;
	while(i<Length)
	{
		if(i<(Length-1))
		Buffer[Length-1]+=Buffer[i];//累加Length-1前的数据
		USART1_send_byte(Buffer[i++]);
	}
}
void USART1_send_byte(uint8_t byte)
{
	while(USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET);//等待发送完成
	USART1->DR=byte;	
}    


#endif	

