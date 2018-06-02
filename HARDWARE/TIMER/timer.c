#include "timer.h"
#include "usart.h"
#include <stdlib.h>//调用atoi函数的头文件
#include "string.h" 
#include "usart3.h"
#include "led.h"
#include "includes.h"	//ucos 使用									  
////////////////////////////////////////////////////////////////////////////////// 	 



//u8 *p2,*p3;
//u8 Server_len=0;
//u8 Server_data[]={0};


 
	 
void TIM3_Int_Init(u16 arr,u16 psc)
{	
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);//TIM7时钟使能    
	
	//定时器TIM7初始化
	TIM_TimeBaseStructure.TIM_Period = arr;                     //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler =psc;                   //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;     //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);             //根据指定的参数初始化TIMx的时间基数单位
 
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE );                   //使能指定的TIM7中断,允许更新中断
	
	TIM_Cmd(TIM3,ENABLE);//开启定时器7
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0 ;//抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//子优先级2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
	TIM_Cmd(TIM3,DISABLE);		//关闭定时器7
}
		    
void TIM3_IRQHandler(void)
{ 
	OSIntEnter(); 
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)//是更新中断
	{	 			   
		   TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );  //清除TIM7更新中断标志    
           
		   USART3_RECEIVE_OK=1;	//标记接收完成
             
		   TIM_Cmd(TIM3, DISABLE);  //关闭TIM7 
//		
//		  p2=(u8*)strstr((const char*)USART3_RX_BUF,"+IPD");
//			if(p2)//接收到TCP/UDP数据
//			{
//				p3=(u8*)strstr((const char*)p2,",");
//				p2=(u8*)strstr((const char*)p2,":");
//				p2[0]=0;//加入结束符

//				Server_len=atoi(p3+1);
////				printf("%d",Server_len);
//				memcpy(Server_data,p2+1,Server_len);//拷贝接收到的数据
//				if(Server_data[0]==49||Server_data[0]==48) {LED0^=1;}
   

	//		}
	
	}
   OSIntExit();  //退出中断，发生一次任务调度	    
}

