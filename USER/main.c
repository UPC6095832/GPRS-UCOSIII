#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "usart3.h"
#include "includes.h"
#include "timer.h"
#include "sim800c.h"
/************************************************
 ALIENTEKս��STM32������UCOSʵ��
 ��4-1 UCOSIII UCOSIII��ֲ
 ����֧�֣�www.openedv.com
 �Ա����̣�http://eboard.taobao.com 
 ��ע΢�Ź���ƽ̨΢�źţ�"����ԭ��"����ѻ�ȡSTM32���ϡ�
 ������������ӿƼ����޹�˾  
 ���ߣ�����ԭ�� @ALIENTEK
************************************************/

//UCOSIII���������ȼ��û�������ʹ�ã�ALIENTEK
//����Щ���ȼ��������UCOSIII��5��ϵͳ�ڲ�����
//���ȼ�0���жϷ������������� OS_IntQTask()
//���ȼ�1��ʱ�ӽ������� OS_TickTask()
//���ȼ�2����ʱ���� OS_TmrTask()
//���ȼ�OS_CFG_PRIO_MAX-2��ͳ������ OS_StatTask()
//���ȼ�OS_CFG_PRIO_MAX-1���������� OS_IdleTask()
//����֧�֣�www.openedv.com
//�Ա����̣�http://eboard.taobao.com  
//������������ӿƼ����޹�˾  
//���ߣ�����ԭ�� @ALIENTEK

/*********************��������************************/

#define START_TASK_PRIO		3     //�������ȼ�	
#define START_STK_SIZE 		512    //�����ջ��С
OS_TCB StartTaskTCB;  //������ƿ� ������¼����ĸ�������
CPU_STK START_TASK_STK[START_STK_SIZE];//�����ջ
void start_task(void *p_arg);//������ �û���д����������룬ʵʵ���ڸɻ��

/*********************USART1����************************/

#define USART1_TASK_PRIO		6
#define USART1_STK_SIZE 		128
OS_TCB USART1TaskTCB;
CPU_STK USART1_TASK_STK[USART1_STK_SIZE];
void usart1_task(void *p_arg);

/*********************USART3����************************/

#define USART3_TASK_PRIO		5	
#define USART3_STK_SIZE 		128
OS_TCB USART3TaskTCB;	
CPU_STK USART3_TASK_STK[USART3_STK_SIZE];
void usart3_task(void *p_arg);

/*********************GPRS����************************/

#define GPRS_TASK_PRIO		4	
#define GPRS_STK_SIZE 		128
OS_TCB GPRSTaskTCB;	
CPU_STK GPRS_TASK_STK[GPRS_STK_SIZE];
void gprs_task(void *p_arg);

/*********************��ʱ��************************/

OS_TMR 	tmr1;		//��ʱ��1
OS_TMR	tmr2;		//��ʱ��2
void tmr1_callback(void *p_tmr, void *p_arg); 	//��ʱ��1�ص�����
void tmr2_callback(void *p_tmr, void *p_arg);	//��ʱ��2�ص�����

/*********************�ź���************************/

OS_SEM	SYNC_SEM;		//����һ���ź�������������ͬ��

int main(void)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	
	delay_init();       //��ʱ��ʼ��
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //�жϷ�������
	uart_init(9600);    //���ڲ���������
    usart3_init(9600);    //���ڲ���������
	LED_Init();         //LED��ʼ��
    TIM3_Int_Init(99,7199);		//10ms�ж�
	send_com(0x81);
    
	OSInit(&err);		//��ʼ��UCOSIII
    
    //OS_CFG_ISR_POST_DEFERRED_EN==1���ж���ʱ���������ø������������ķ����������ٽ���롣
	OS_CRITICAL_ENTER();//�����ٽ�����ʹ��ǰ�������CPU_SR_ALLOC();
                        //ֻ�е������������ж���ʹ�ܵģ�����ڴ����ٽ��ʱ�жϷ�����
                        //ISR����ͻᱻִ�С���ISR ��ĩβ��uC/OS-III�᷵��ԭ���񣨼�ʹISR���и����ȼ����񱻾�������
	/*
    1.uC/OS-III��ͨ��������ƿ������������,��˴�������Ĺ���ʵ�����Ǹ�����Ĵ������һ��������ƿ�,
    ��ͨ��������ƿ���������������ջ���������γ�һ������������.��Ȼ��Ҫʹ�մ���������������״̬,
    ����������һ���������.
    2.OSTaskCreate ()������Ҫ���3���:�����ջ�ĳ�ʼ��,������ƿ�ĳ�ʼ�����������.
    */
	OSTaskCreate((OS_TCB 	* )&StartTaskTCB,		//������ƿ�
				 (CPU_CHAR	* )"start task", 		//��������
                 (OS_TASK_PTR )start_task, 			//������
                 (void		* )0,					//���ݸ��������Ĳ���
                 (OS_PRIO	  )START_TASK_PRIO,     //�������ȼ�
                 (CPU_STK   * )&START_TASK_STK[0],	//�����ջ����ַ
                 (CPU_STK_SIZE)START_STK_SIZE/10,	//�����ջ�����λ
                 (CPU_STK_SIZE)START_STK_SIZE,		//�����ջ��С
                 (OS_MSG_QTY  )0,					//�����ڲ���Ϣ�����ܹ����յ������Ϣ��Ŀ,Ϊ0ʱ��ֹ������Ϣ
                 (OS_TICK	  )0,					//��ʹ��ʱ��Ƭ��תʱ��ʱ��Ƭ���ȣ�Ϊ0ʱΪĬ�ϳ��ȣ�
                 (void   	* )0,					//�û�����Ĵ洢��
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, //����ѡ��
                 (OS_ERR 	* )&err);				//��Ÿú�������ʱ�ķ���ֵ
	OS_CRITICAL_EXIT();	//�˳��ٽ���	 
	OSStart(&err);  //����UCOSIII
	while(1);
}
//��ʼ������
void start_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();//����洢�ռ�洢��ǰcpu���ж�״̬
	p_arg = p_arg;

	CPU_Init();
#if OS_CFG_STAT_TASK_EN > 0u
   OSStatTaskCPUUsageInit(&err);  	//ͳ������                
#endif
	
#ifdef CPU_CFG_INT_DIS_MEAS_EN		//���ʹ���˲����жϹر�ʱ��
    CPU_IntDisMeasMaxCurReset();	
#endif 
	
#if	OS_CFG_SCHED_ROUND_ROBIN_EN  //��ʹ��ʱ��Ƭ��ת��ʱ��
	 //ʹ��ʱ��Ƭ��ת���ȹ���,ʱ��Ƭ����Ϊ1��ϵͳʱ�ӽ��ģ���1*5=5ms
	OSSchedRoundRobinCfg(DEF_ENABLED,1,&err);  
#endif		
	
	OS_CRITICAL_ENTER();	//�����ٽ��������ٽ籣����ʱ��һ��Ҫ����CPU_SR_ALLOC();
    //����һ���ź���
	OSSemCreate ((OS_SEM*	)&SYNC_SEM,
                 (CPU_CHAR*	)"SYNC_SEM",
                 (OS_SEM_CTR)0,		
                 (OS_ERR*	)&err);
	//����LED0����
	OSTaskCreate((OS_TCB 	* )&USART1TaskTCB,		
				 (CPU_CHAR	* )"usart1 task", 		
                 (OS_TASK_PTR )usart1_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )USART1_TASK_PRIO,     
                 (CPU_STK   * )&USART1_TASK_STK[0],	
                 (CPU_STK_SIZE)USART1_STK_SIZE/10,	
                 (CPU_STK_SIZE)USART1_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,	//����ʱ��Ƭ10ms				
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);				
				 
	//����LED1����
	OSTaskCreate((OS_TCB 	* )&USART3TaskTCB,		
				 (CPU_CHAR	* )"usart3 task", 		
                 (OS_TASK_PTR )usart3_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )USART3_TASK_PRIO,     	
                 (CPU_STK   * )&USART3_TASK_STK[0],	
                 (CPU_STK_SIZE)USART3_STK_SIZE/10,	
                 (CPU_STK_SIZE)USART3_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,	//����ʱ��Ƭ10ms				
                 (void   	* )0,				
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, 
                 (OS_ERR 	* )&err);

//����GPRS����
	OSTaskCreate((OS_TCB 	* )&GPRSTaskTCB,		
				 (CPU_CHAR	* )"gprs task", 		
                 (OS_TASK_PTR )gprs_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )GPRS_TASK_PRIO,     	
                 (CPU_STK   * )&GPRS_TASK_STK[0],	
                 (CPU_STK_SIZE)GPRS_STK_SIZE/10,	
                 (CPU_STK_SIZE)GPRS_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,	//����ʱ��Ƭ10ms				
                 (void   	* )0,				
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, 
                 (OS_ERR 	* )&err);	                 
			 
	//������ʱ��1
	OSTmrCreate((OS_TMR		*)&tmr1,		//��ʱ��1
                (CPU_CHAR	*)"tmr1",		//��ʱ������
                (OS_TICK	 )0,			//20*10=200ms
                (OS_TICK	 )300,          //100*10=1000ms
                (OS_OPT		 )OS_OPT_TMR_PERIODIC, //����ģʽ
                (OS_TMR_CALLBACK_PTR)tmr1_callback,//��ʱ��1�ص�����
                (void	    *)0,			//����Ϊ0
                (OS_ERR	    *)&err);		//���صĴ�����
				
				
	//������ʱ��2
	OSTmrCreate((OS_TMR		*)&tmr2,		
                (CPU_CHAR	*)"tmr2",		
                (OS_TICK	 )200,			//200*10=2000ms	
                (OS_TICK	 )0,   					
                (OS_OPT		 )OS_OPT_TMR_ONE_SHOT, 	//���ζ�ʱ��
                (OS_TMR_CALLBACK_PTR)tmr2_callback,	//��ʱ��2�ص�����
                (void	    *)0,			
                (OS_ERR	    *)&err);						 
					 
	OS_CRITICAL_EXIT();	//�˳��ٽ���
	OSTaskDel((OS_TCB*)&StartTaskTCB,&err);	//ɾ��start_task��������
} 



//���ڴ���GY39����
void usart1_task(void *p_arg)
{
	u8 sum=0,i=0;
    uint32_t lux=0;
	OS_ERR err;
    CPU_TS ts;
	p_arg = p_arg;

	while(1)
	{
   
       
     	if(USART1_RECEIVE_OK)//���ڽ������
		{
			for(sum=0,i=0;i<(USART1_RX_BUF[3]+4);i++)//rgb_data[3]=3
			sum+=USART1_RX_BUF[i];
			if(sum==USART1_RX_BUF[i])//У����ж�
			{

                lux=((USART1_RX_BUF[4]<<24)|(USART1_RX_BUF[5]<<16)|(USART1_RX_BUF[6]<<8)|USART1_RX_BUF[7])/100;
                u3_printf("����:%d\r\n",lux);
             
			}
            
			USART1_RECEIVE_OK=0;//����������ϱ�־
		}
        
        
     OSTimeDlyHMSM(0,0,0,300,OS_OPT_TIME_HMSM_STRICT,&err);
	}
} 
//���ڴ��������ָ��
void usart3_task(void *p_arg)
{
	u8 t;
	OS_ERR err;
	p_arg = p_arg;
    
	while(1)
	{
    	if(USART3_RECEIVE_OK==1)
		{					   
	      /*��ӡ�������յ������ݷ���һ		
			for(t=0;t<USART3_RX_STA;t++)
			{
				USART_SendData(USART3, USART3_RX_BUF[t]);//�򴮿�1��������
				while(USART_GetFlagStatus(USART3,USART_FLAG_TC)!=SET);//�ȴ����ͽ���
			}
          */
            
          /*��ӡ�������յ������ݷ����� 
           USART3_RX_BUF[USART3_RX_STA]=0;//��ӽ�����
		   u3_printf("%s",USART3_RX_BUF);	//���͵�����           
         */
            
           USART3_RX_STA=0;
		   USART3_RECEIVE_OK=0;
		}
      OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_HMSM_STRICT,&err);	
	}
}
//����
void gprs_task(void *p_arg)
{
	
	OS_ERR err;
	p_arg = p_arg; 
    gprs_join_server();
  
	while(1)
	{
  
        
       
      OSTimeDlyHMSM(0,0,2,0,OS_OPT_TIME_HMSM_STRICT,&err);	
	}
}

//��ʱ��1�Ļص�����
void tmr1_callback(void *p_tmr, void *p_arg)
{
      OS_ERR err;

    


}

//��ʱ��2�Ļص�����
void tmr2_callback(void *p_tmr,void *p_arg)
{


}



