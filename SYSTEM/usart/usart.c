#include "sys.h"
#include "usart.h"	  
#include "can.h"
#include "string.h"


//����1
//֧����Ӧ��ͬƵ���µĴ��ڲ���������.
//�����˶�printf��֧��
//�����˴��ڽ��������.
//������printf��һ���ַ���ʧ��bug

//V1.4�޸�˵��
//1,�޸Ĵ��ڳ�ʼ��IO��bug
//2,�޸���USART_RX_STA,ʹ�ô����������ֽ���Ϊ2��14�η�
//3,������USART_REC_LEN,���ڶ��崮�����������յ��ֽ���(������2��14�η�)
//4,�޸���EN_USART1_RX��ʹ�ܷ�ʽ

//V1.5�޸�˵��
//1,�����˶�UCOSII��֧��  

//////////////////////////////////////////////////////////////////
//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  

#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
int _sys_exit(int x) 
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

//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
int  USART_RX_BUF[16]={0};     //���ջ���,���USART_REC_LEN���ֽ�.
int  USART_TX_BUF[16]={0};     //���ջ���,���USART_REC_LEN���ֽ�.
int  data[16]={0};//ʵ�����ݴ洢

extern int Can_TX_Buff[16];
u8 count=0;
u8 USART_RX_STA=0;       //����״̬���	  
  
void uart_init(u32 bound)
	{
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
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;//ѡ���ж�ͨ��
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

//ʹ���Զ���Э�����ʮ����������
//====================================�����жϷ������=================================================

/*--------------------------------����Э��-----------------------------------
//---------------- 5a CAN_ID DATA_len 00 00 00 00 00 5b ----------------------
//����ͷ0x5a + CAN_ID + ���� + ����β0d5b ���� �Ÿ��ֽ�
--------------------------------------------------------------------------*/
//�жϺ��� �жϣ�һ���ֽ�(8λ)�Ľ���ÿ�յ�һ���ֽڲ���һ���ж�
//DMA:������Ϊһ֡һ֡�Ľ��գ�һ֡���ݽ��������ٲ����ж�
void USART1_IRQHandler(void) 
{
  u8 i;
	if(USART_GetITStatus(USART1,USART_IT_RXNE)!= RESET)//���ڽ��յ�����
	{
		USART_ClearITPendingBit(USART1,USART_IT_RXNE);//��������жϱ�־λ
		//���������յ�������
	  USART_RX_BUF[count++]=USART_ReceiveData(USART1);//�Ѵ���2�����ݸ���SBUS
		
		if(USART_RX_BUF[0]==0x5a&&count==9&&USART_RX_BUF[8]==0x5b)//���֡ͷ֡β�ͳ������� ����
		{
		  count=0;//���ճ������� �ȴ��´θ�ֵ
			//�������� �����ټ�
			memset(data,0,sizeof(data)); //��������
			//���ݴ������ֵת�Ƶ�����������
			for(i=0;i<9;i++)
			{
			  data[i]=USART_RX_BUF[i];//���յ����ݱ��浽data
				printf("��%d=%X\r\n",i,data[i]);
			}
			USART_RX_STA=1;//��������ȷ������
		}
	}
}

/*--------------------------------����Э��-----------------------------------
//---------------- 5a CAN_ID DATA_len 00 00 00 00 00 5b ----------------------
//����ͷ0x5a + CAN_ID + ���ݳ��� +0X02ת�ٿ��� ����β0d5b ���� �Ÿ��ֽ�
--------------------------------------------------------------------------*/
//void send_cmd(u16* data)//����������ת����CAN
//{
//	u8 i=0;
//	CanTxMsg TxMessage;  
//  TxMessage.StdId=data[1];	//��׼��ʶ��
//	TxMessage.ExtId=data[1]; //��չ��ʶ��
//  TxMessage.IDE=CAN_Id_Standard;//ʹ�ñ�׼��ʶ��
//  TxMessage.RTR=CAN_RTR_DATA;//Ϊ����֡
//  TxMessage.DLC=data[2];	//	��Ϣ�����ݳ���
//	
//		for(i=0;i<data[2];i++)//ȡ��֡ͷ ֡β
//	{
//		TxMessage.Data[i]=data[i+3]; //��һ���ֽ����� 
//		printf("�������ݣ�%X",TxMessage.Data[i]);
//	}
//	
//	CAN_Transmit(CAN1,&TxMessage); //��������
//}

void send_cmd(int* data)//����������ת����CAN
{
	u8 i=0;
	CanTxMsg TxMessage;  
  TxMessage.StdId=data[1];	//��׼��ʶ��Ϊ0x00
  TxMessage.ExtId=data[1]; //��չ��ʶ��0x0000
  TxMessage.IDE=CAN_Id_Standard;//ʹ�ñ�׼��ʶ��
  TxMessage.RTR=CAN_RTR_DATA;//Ϊ����֡
  TxMessage.DLC=data[2];	//	��Ϣ�����ݳ��� 2
	for(i=0;i<5;i++)//ȡ��֡ͷ ֡β
	{
		TxMessage.Data[i]=data[i+3]; //��һ���ֽ����� 
		if(i>=data[2])
		{
			break;//��������
		}
	}
	CAN_Transmit(CAN1,&TxMessage); //��������
}

//ͨ�������ⷢCAN�յ�����Ϣ
void send_report(int* data)
{
	u8 i,k;
	u8 data_buf[16]={0};
	data_buf[0]=0x5a;
	data_buf[8]=0x5b;
	for(i=1;i<8;i++)
	{
		data_buf[i]=data[i-1];
	}
	for(k=0;k<9;k++)
	{
		USART_SendByte(data_buf[k]);
	}
}

//���������ֽ����ݺ���
void Usart_SendHalfWord(USART_TypeDef* pUSARTx,uint16_t data)
{
	//����ʮ��λ����Ҫ��Ϊ���������ͣ��ȶ�����������
	uint8_t temp_h,temp_l;//����8λ�ı������ֱ�洢��8λ�͵�8λ��

	//����ȡ����8λ
	temp_h=(data&0xff00)>>8;//�Ͱ�λ����0��&����8λ��Ϊ0������8λ��0xff00��16λ�����ƣ�
	//��ȡ����8λ
	temp_l=data&0xff;//ȡ����8λ����
	//16λ�����������Ӿͷŵ��������������棨��16λ��
	
	//���ù̼��⺯��
	USART_SendData(pUSARTx,temp_h);//�ȷ��͸�8λ
	while(USART_GetFlagStatus(pUSARTx,USART_FLAG_TXE)==RESET);//�ȴ����ݷ������

	USART_SendData(pUSARTx,temp_l);//�ٷ��͵�8λ
	while(USART_GetFlagStatus(pUSARTx,USART_FLAG_TXE)==RESET);//�ȴ����ݷ������

}

/**************************************************************************
�������ܣ�����һ���ֽ�
��ڲ��������͵��ֽ�
����  ֵ����
**************************************************************************/
void USART_SendByte(uint8_t Byte)
{
	USART_SendData(USART1,Byte);
	// �ȴ�д����ɣ�д�����֮��Ὣ��־λ�Զ���0
	while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET);
}

/**************************************************************************
�������ܣ�����ָ����С������
��ڲ����������ַ�������С
����  ֵ����
**************************************************************************/
void USART_SendArray(uint8_t *Array,uint16_t Length)
	{
	uint8_t i = 0;
	for(i=0;i<Length;i++)
		{
		USART_SendData(USART1,Array[i]);
		// �ȴ�д����ɣ�д�����֮��Ὣ��־λ�Զ���0
		while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET);
   	}
  }

/**************************************************************************
�������ܣ�����ָ����С���ַ����飬��usartSendData��������
��ڲ����������ַ�������С
����  ֵ����
**************************************************************************/
void USART_Send_String(u8 *p,u16 sendSize)
{ 
	static int length =0;
	while(length<sendSize)
	{   
		//�����ʹ�ò���USART1���ĳ���Ӧ�ģ�����USART3�������������޸�
		while( !(USART1->SR&(0x01<<7)) );//���ͻ�����Ϊ��
		USART1->DR=*p;                   
		p++;
		length++;
	}
	length =0;
}

void usart1_send(unsigned char *data,unsigned char len)
{
		unsigned char t;
		for(t=0;t<len;t++)
        {	
		    while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);
            USART_SendData(USART1,data[t]); 				
        }
}
/**************************************************************************
�������ܣ������ת�ٵ���Ϣ���д����ͨ�����ڷ��͸�Linux
��ڲ�����CAN��������
����  ֵ����
**************************************************************************/
//void usartSendData(char *Can_RX_Buff)
//{
//	//Э�����ݻ�������
//  char buf[20]={0};
//	int i;
//	//֡ͷ
//	buf[0]=0x55;
//	buf[1]=0xaa;
//	//���÷�������

//	for(i=2;i<20;i++)
//	{
//		buf[i]=Can_RX_Buff[i];                   // buf[2]data
//	}
//	//�����ַ�������
////	USART_Send_String(buf,sizeof(buf));
//   
//	for(i=0;i<20;i++)
//	{
//		USART_SendData(USART1,buf[i]);
//		printf("���͸�CAN������%c ",buf[i]);
//	}
//}

/**********************************END***************************************/
