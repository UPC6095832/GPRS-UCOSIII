#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "usart3.h"
#include "includes.h"
#include "timer.h"
#include "sim800c.h"
/************************************************
 ALIENTEK战舰STM32开发板UCOS实验
 例4-1 UCOSIII UCOSIII移植
 技术支持：www.openedv.com
 淘宝店铺：http://eboard.taobao.com 
 关注微信公众平台微信号："正点原子"，免费获取STM32资料。
 广州市星翼电子科技有限公司  
 作者：正点原子 @ALIENTEK
************************************************/

//UCOSIII中以下优先级用户程序不能使用，ALIENTEK
//将这些优先级分配给了UCOSIII的5个系统内部任务
//优先级0：中断服务服务管理任务 OS_IntQTask()
//优先级1：时钟节拍任务 OS_TickTask()
//优先级2：定时任务 OS_TmrTask()
//优先级OS_CFG_PRIO_MAX-2：统计任务 OS_StatTask()
//优先级OS_CFG_PRIO_MAX-1：空闲任务 OS_IdleTask()
//技术支持：www.openedv.com
//淘宝店铺：http://eboard.taobao.com  
//广州市星翼电子科技有限公司  
//作者：正点原子 @ALIENTEK

/*********************启动任务************************/

#define START_TASK_PRIO		3     //任务优先级	
#define START_STK_SIZE 		512    //任务堆栈大小
OS_TCB StartTaskTCB;  //任务控制块 用来记录任务的各个属性
CPU_STK START_TASK_STK[START_STK_SIZE];//任务堆栈
void start_task(void *p_arg);//任务函数 用户编写的任务处理代码，实实在在干活的

/*********************USART1任务************************/

#define USART1_TASK_PRIO		6
#define USART1_STK_SIZE 		128
OS_TCB USART1TaskTCB;
CPU_STK USART1_TASK_STK[USART1_STK_SIZE];
void usart1_task(void *p_arg);

/*********************USART3任务************************/

#define USART3_TASK_PRIO		5	
#define USART3_STK_SIZE 		128
OS_TCB USART3TaskTCB;	
CPU_STK USART3_TASK_STK[USART3_STK_SIZE];
void usart3_task(void *p_arg);

/*********************GPRS任务************************/

#define GPRS_TASK_PRIO		4	
#define GPRS_STK_SIZE 		128
OS_TCB GPRSTaskTCB;	
CPU_STK GPRS_TASK_STK[GPRS_STK_SIZE];
void gprs_task(void *p_arg);

/*********************定时器************************/

OS_TMR 	tmr1;		//定时器1
OS_TMR	tmr2;		//定时器2
void tmr1_callback(void *p_tmr, void *p_arg); 	//定时器1回调函数
void tmr2_callback(void *p_tmr, void *p_arg);	//定时器2回调函数

/*********************信号量************************/

OS_SEM	SYNC_SEM;		//定义一个信号量，用于任务同步

int main(void)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	
	delay_init();       //延时初始化
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //中断分组配置
	uart_init(9600);    //串口波特率设置
    usart3_init(9600);    //串口波特率设置
	LED_Init();         //LED初始化
    TIM3_Int_Init(99,7199);		//10ms中断
	send_com(0x81);
    
	OSInit(&err);		//初始化UCOSIII
    
    //OS_CFG_ISR_POST_DEFERRED_EN==1（中断延时发布）采用给调度器上锁的方法来保护临界代码。
	OS_CRITICAL_ENTER();//进入临界区，使用前必须调用CPU_SR_ALLOC();
                        //只有调度器被锁，中断是使能的，如果在处理临界段时中断发生，
                        //ISR程序就会被执行。在ISR 的末尾，uC/OS-III会返回原任务（即使ISR中有高优先级任务被就绪）。
	/*
    1.uC/OS-III是通过任务控制块来管理任务的,因此创建任务的工作实质上是给任务的代码分配一个任务控制块,
    并通过任务控制块把任务代码和任务堆栈关联起来形成一个完整的任务.当然还要使刚创建的任务进入就绪状态,
    并接着引发一次任务调度.
    2.OSTaskCreate ()函数主要完成3项工作:任务堆栈的初始化,任务控制块的初始化和任务调度.
    */
	OSTaskCreate((OS_TCB 	* )&StartTaskTCB,		//任务控制块
				 (CPU_CHAR	* )"start task", 		//任务名字
                 (OS_TASK_PTR )start_task, 			//任务函数
                 (void		* )0,					//传递给任务函数的参数
                 (OS_PRIO	  )START_TASK_PRIO,     //任务优先级
                 (CPU_STK   * )&START_TASK_STK[0],	//任务堆栈基地址
                 (CPU_STK_SIZE)START_STK_SIZE/10,	//任务堆栈深度限位
                 (CPU_STK_SIZE)START_STK_SIZE,		//任务堆栈大小
                 (OS_MSG_QTY  )0,					//任务内部消息队列能够接收的最大消息数目,为0时禁止接收消息
                 (OS_TICK	  )0,					//当使能时间片轮转时的时间片长度，为0时为默认长度，
                 (void   	* )0,					//用户补充的存储区
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, //任务选项
                 (OS_ERR 	* )&err);				//存放该函数错误时的返回值
	OS_CRITICAL_EXIT();	//退出临界区	 
	OSStart(&err);  //开启UCOSIII
	while(1);
}
//开始任务函数
void start_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();//分配存储空间存储当前cpu的中断状态
	p_arg = p_arg;

	CPU_Init();
#if OS_CFG_STAT_TASK_EN > 0u
   OSStatTaskCPUUsageInit(&err);  	//统计任务                
#endif
	
#ifdef CPU_CFG_INT_DIS_MEAS_EN		//如果使能了测量中断关闭时间
    CPU_IntDisMeasMaxCurReset();	
#endif 
	
#if	OS_CFG_SCHED_ROUND_ROBIN_EN  //当使用时间片轮转的时候
	 //使能时间片轮转调度功能,时间片长度为1个系统时钟节拍，既1*5=5ms
	OSSchedRoundRobinCfg(DEF_ENABLED,1,&err);  
#endif		
	
	OS_CRITICAL_ENTER();	//进入临界区，用临界保护的时候一定要调用CPU_SR_ALLOC();
    //创建一个信号量
	OSSemCreate ((OS_SEM*	)&SYNC_SEM,
                 (CPU_CHAR*	)"SYNC_SEM",
                 (OS_SEM_CTR)0,		
                 (OS_ERR*	)&err);
	//创建LED0任务
	OSTaskCreate((OS_TCB 	* )&USART1TaskTCB,		
				 (CPU_CHAR	* )"usart1 task", 		
                 (OS_TASK_PTR )usart1_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )USART1_TASK_PRIO,     
                 (CPU_STK   * )&USART1_TASK_STK[0],	
                 (CPU_STK_SIZE)USART1_STK_SIZE/10,	
                 (CPU_STK_SIZE)USART1_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,	//两个时间片10ms				
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);				
				 
	//创建LED1任务
	OSTaskCreate((OS_TCB 	* )&USART3TaskTCB,		
				 (CPU_CHAR	* )"usart3 task", 		
                 (OS_TASK_PTR )usart3_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )USART3_TASK_PRIO,     	
                 (CPU_STK   * )&USART3_TASK_STK[0],	
                 (CPU_STK_SIZE)USART3_STK_SIZE/10,	
                 (CPU_STK_SIZE)USART3_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,	//两个时间片10ms				
                 (void   	* )0,				
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, 
                 (OS_ERR 	* )&err);

//创建GPRS任务
	OSTaskCreate((OS_TCB 	* )&GPRSTaskTCB,		
				 (CPU_CHAR	* )"gprs task", 		
                 (OS_TASK_PTR )gprs_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )GPRS_TASK_PRIO,     	
                 (CPU_STK   * )&GPRS_TASK_STK[0],	
                 (CPU_STK_SIZE)GPRS_STK_SIZE/10,	
                 (CPU_STK_SIZE)GPRS_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,	//两个时间片10ms				
                 (void   	* )0,				
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, 
                 (OS_ERR 	* )&err);	                 
			 
	//创建定时器1
	OSTmrCreate((OS_TMR		*)&tmr1,		//定时器1
                (CPU_CHAR	*)"tmr1",		//定时器名字
                (OS_TICK	 )0,			//20*10=200ms
                (OS_TICK	 )300,          //100*10=1000ms
                (OS_OPT		 )OS_OPT_TMR_PERIODIC, //周期模式
                (OS_TMR_CALLBACK_PTR)tmr1_callback,//定时器1回调函数
                (void	    *)0,			//参数为0
                (OS_ERR	    *)&err);		//返回的错误码
				
				
	//创建定时器2
	OSTmrCreate((OS_TMR		*)&tmr2,		
                (CPU_CHAR	*)"tmr2",		
                (OS_TICK	 )200,			//200*10=2000ms	
                (OS_TICK	 )0,   					
                (OS_OPT		 )OS_OPT_TMR_ONE_SHOT, 	//单次定时器
                (OS_TMR_CALLBACK_PTR)tmr2_callback,	//定时器2回调函数
                (void	    *)0,			
                (OS_ERR	    *)&err);						 
					 
	OS_CRITICAL_EXIT();	//退出临界区
	OSTaskDel((OS_TCB*)&StartTaskTCB,&err);	//删除start_task任务自身
} 



//用于处理GY39数据
void usart1_task(void *p_arg)
{
	u8 sum=0,i=0;
    uint32_t lux=0;
	OS_ERR err;
    CPU_TS ts;
	p_arg = p_arg;

	while(1)
	{
   
       
     	if(USART1_RECEIVE_OK)//串口接收完毕
		{
			for(sum=0,i=0;i<(USART1_RX_BUF[3]+4);i++)//rgb_data[3]=3
			sum+=USART1_RX_BUF[i];
			if(sum==USART1_RX_BUF[i])//校验和判断
			{

                lux=((USART1_RX_BUF[4]<<24)|(USART1_RX_BUF[5]<<16)|(USART1_RX_BUF[6]<<8)|USART1_RX_BUF[7])/100;
                u3_printf("光照:%d\r\n",lux);
             
			}
            
			USART1_RECEIVE_OK=0;//处理数据完毕标志
		}
        
        
     OSTimeDlyHMSM(0,0,0,300,OS_OPT_TIME_HMSM_STRICT,&err);
	}
} 
//用于处理服务器指令
void usart3_task(void *p_arg)
{
	u8 t;
	OS_ERR err;
	p_arg = p_arg;
    
	while(1)
	{
    	if(USART3_RECEIVE_OK==1)
		{					   
	      /*打印出来接收到的数据方法一		
			for(t=0;t<USART3_RX_STA;t++)
			{
				USART_SendData(USART3, USART3_RX_BUF[t]);//向串口1发送数据
				while(USART_GetFlagStatus(USART3,USART_FLAG_TC)!=SET);//等待发送结束
			}
          */
            
          /*打印出来接收到的数据方法二 
           USART3_RX_BUF[USART3_RX_STA]=0;//添加结束符
		   u3_printf("%s",USART3_RX_BUF);	//发送到串口           
         */
            
           USART3_RX_STA=0;
		   USART3_RECEIVE_OK=0;
		}
      OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_HMSM_STRICT,&err);	
	}
}
//用于
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

//定时器1的回调函数
void tmr1_callback(void *p_tmr, void *p_arg)
{
      OS_ERR err;

    


}

//定时器2的回调函数
void tmr2_callback(void *p_tmr,void *p_arg)
{


}



