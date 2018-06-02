#include "sys.h"
#include "usart.h"	
#include "string.h"
#include "mytask.h"
#include "includes.h"	//ucos ʹ��	
	  

/**************�������´���,֧��printf����***************/	  
#if 1
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
_sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//ѭ������,ֱ���������   
    USART1->DR = (u8) ch;      
	return ch;
}
#endif 

#if EN_USART1_RX   //���ʹ���˽���
 	
u8 USART1_RX_BUF[USART1_RX_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
u8 USART1_RX_STA=0;       //����״̬���	 
u8 USART1_RECEIVE_OK=0;  

void uart_init(u32 bound){
  //GPIO�˿�����
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
	 
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//ʹ��USART1��GPIOAʱ��
  
	//USART1_TX   GPIOA.9
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
  GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.9
   
  //USART1_RX	  GPIOA.10��ʼ��
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
  GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.10  

  //Usart1 NVIC ����
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
  
   //USART ��ʼ������

	USART_InitStructure.USART_BaudRate = bound;//���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

  USART_Init(USART1, &USART_InitStructure); //��ʼ������1
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//�������ڽ����ж�
  USART_Cmd(USART1, ENABLE);                    //ʹ�ܴ���1 

}

void USART1_IRQHandler(void)                	//����1�жϷ������
	{
    OS_ERR err;   
	OSIntEnter();    

	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)//�жϽ��ձ�־
	{
		USART1_RX_BUF[USART1_RX_STA++]=USART_ReceiveData(USART1);//��ȡ�������ݣ�ͬʱ����ձ�־
		if (USART1_RX_BUF[0]!=0x5a)//֡ͷ����
			USART1_RX_STA=0;	
		if ((USART1_RX_STA==2)&&(USART1_RX_BUF[1]!=0x5a))//֡ͷ����
			USART1_RX_STA=0;
	
		if(USART1_RX_STA>3)//i����4ʱ���Ѿ����յ��������ֽ�rebuf[3]
		{
			if(USART1_RX_STA==(USART1_RX_BUF[3]+5))//�ж��Ƿ����һ֡�������
                
            {

			switch(USART1_RX_BUF[2])//������Ϻ���
			{
				case 0x45:
					if(!USART1_RECEIVE_OK)//�����ݴ�����ɺ�Ž����µ�����
					{
                      
						USART1_RECEIVE_OK=1;//������ɱ�־
					}
					break;
				case 0x15:
                    if(!USART1_RECEIVE_OK)//�����ݴ�����ɺ�Ž����µ�����
					{
                       
						USART1_RECEIVE_OK=1;//������ɱ�־
					}
                    break;//ԭʼ���ݽ��գ���ģ��0x45�ķ�ʽ
				case 0x35:break;
			}
              USART1_RX_STA=0;//������0  
            }
			
		}
	}
        

	OSIntExit();  //�˳��жϣ�����һ���������											 

} 
    
void send_com(u8 data)
{
	u8 bytes[3]={0};
	bytes[0]=0xa5;
	bytes[1]=data;//�����ֽ�
	USART_Send(bytes,3);//����֡ͷ�������ֽڡ�У���
}
void USART_Send(uint8_t *Buffer, uint8_t Length)
{
	uint8_t i=0;
	while(i<Length)
	{
		if(i<(Length-1))
		Buffer[Length-1]+=Buffer[i];//�ۼ�Length-1ǰ������
		USART1_send_byte(Buffer[i++]);
	}
}
void USART1_send_byte(uint8_t byte)
{
	while(USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET);//�ȴ��������
	USART1->DR=byte;	
}    


#endif	

