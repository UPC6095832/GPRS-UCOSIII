#include "timer.h"
#include "usart.h"
#include <stdlib.h>//����atoi������ͷ�ļ�
#include "string.h" 
#include "usart3.h"
#include "led.h"
#include "includes.h"	//ucos ʹ��									  
////////////////////////////////////////////////////////////////////////////////// 	 



//u8 *p2,*p3;
//u8 Server_len=0;
//u8 Server_data[]={0};


 
	 
void TIM3_Int_Init(u16 arr,u16 psc)
{	
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);//TIM7ʱ��ʹ��    
	
	//��ʱ��TIM7��ʼ��
	TIM_TimeBaseStructure.TIM_Period = arr;                     //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	
	TIM_TimeBaseStructure.TIM_Prescaler =psc;                   //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;     //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);             //����ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
 
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE );                   //ʹ��ָ����TIM7�ж�,��������ж�
	
	TIM_Cmd(TIM3,ENABLE);//������ʱ��7
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0 ;//��ռ���ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//�����ȼ�2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
	TIM_Cmd(TIM3,DISABLE);		//�رն�ʱ��7
}
		    
void TIM3_IRQHandler(void)
{ 
	OSIntEnter(); 
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)//�Ǹ����ж�
	{	 			   
		   TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );  //���TIM7�����жϱ�־    
           
		   USART3_RECEIVE_OK=1;	//��ǽ������
             
		   TIM_Cmd(TIM3, DISABLE);  //�ر�TIM7 
//		
//		  p2=(u8*)strstr((const char*)USART3_RX_BUF,"+IPD");
//			if(p2)//���յ�TCP/UDP����
//			{
//				p3=(u8*)strstr((const char*)p2,",");
//				p2=(u8*)strstr((const char*)p2,":");
//				p2[0]=0;//���������

//				Server_len=atoi(p3+1);
////				printf("%d",Server_len);
//				memcpy(Server_data,p2+1,Server_len);//�������յ�������
//				if(Server_data[0]==49||Server_data[0]==48) {LED0^=1;}
   

	//		}
	
	}
   OSIntExit();  //�˳��жϣ�����һ���������	    
}

