#include "sys.h"
#include "sbus.h"
#include "usart.h"
#include "string.h"
#include "led.h"
/*
���ݸ�ʽ
[startbyte]	[data1]	[data2]	��	[data22]	[flags]	[endbyte]
0x0f						                                     0x00
100k�����ʣ�8λ����λ����stm32��Ҫѡ��9λ����żУ�飨EVEN)��2λֹͣλ���޿�����25���ֽڡ�

contrl_flag��־λ����������������px4�Ƿ�Ͽ��ı�־λ��
contrl_flag=1����������������������ӣ�
contrl_flag=0����������������Ͽ���ʧ�أ���px4����Ƶ��ͣת��
*/

//����2 ����ջ�ͨѶ
//Ӳ���ӿ� TX:PA2  RX:PA3
u8 contrl_flag=1;//�ж�ң�����Ƿ�����
u8 RC_LEN=0;//���������ֳ�
u16 sBUF[25];//����buf
u16 Data[25];//�洢BUF

u16 data_22[22];//����ת������ ��������λ
unsigned char	data_b[176];//�����ƴ洢

//ʵ��ֻ������ͨ��
int data_ch[16]={0};//���մ洢 �з�����
static uint16_t CH[6]={0};

int rt[16]={0};
uint16_t values[16]={0};

u8 uart2RxFlag;//����2����״̬��־λ
u8 ChannelDataChangeFlg=1;//ͨ��1�����ݱ�־λ

int lastdata_ch[16]={0};//���մ洢


//���ʾ��Ǵ��ڳ���  8������λ żУ�� 2��ֹͣλ 100K
void sbus_init(u32 bound)//SBUS������Ҫ��100K 
{
	//�ṹ������
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	//����2
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	//USART1_TX   PA.2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA.2
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	//USART1_RX	  PA.3
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;//PA.3
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
  GPIO_Init(GPIOA, &GPIO_InitStructure);  
	//NVIC���� 2 2
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2 ;//��ռ���ȼ�2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =2;		//�����ȼ�2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);

  //USART ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;//���ò�����; 100K
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_2;//2��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_Even;//żУ��
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
 
  USART_Init(USART2, &USART_InitStructure); //��ʼ������
	//Receive Data register not empty interrupt
  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//Enables  the specified USART interrupts.
  USART_Cmd(USART2, ENABLE);                    //ʹ�ܴ��� 
}


//���ڵ������ֲ��ܽ��յ�����ʲô���ݣ�����ת��Ϊ�ַ�
//ֻ�з���ʮ���������ݣ���������ʹ��ʮ��������ʽ��������ʱ�Ų����ַ�
//����2�жϷ������ ����SBUS����
//�о�һ���ǽ���һ֡
void USART2_IRQHandler(void) 
{
  u8 i;
	if(USART_GetITStatus(USART2,USART_IT_RXNE)!= RESET)//���ڽ��յ�����
	{
		USART_ClearITPendingBit(USART2,USART_IT_RXNE);//��������жϱ�־λ
		//���������յ�������
	  sBUF[RC_LEN++]=USART_ReceiveData(USART2);//�Ѵ���2�����ݸ���SBUS
		
		if(sBUF[0]==0x0f&&RC_LEN==24&&sBUF[24]==0x00)//���֡ͷ֡β�ͳ������� ����
		{
		  RC_LEN=0;//���ճ������� �ȴ��´θ�ֵ
			//�������� �����ټ�
			memset(Data,0,sizeof(Data)); //��������
				if(i==0) 
				{
					uart2RxFlag=0;//��־λ����
				}
			//���ݴ������ֵת�Ƶ�����������
			for(i=0;i<25;i++)
			{
			  Data[i]=sBUF[i];
			}
			uart2RxFlag=1;//��������ȷ������
			contrl_flag=Data[23];//��־λ
		}
	}
}

////�ѽ��յ����ݷŵ�Data[i]������
//void pre_decode(void)
//{
//	  u8 i;
//		if(sBUF[0]==0x0f&&RC_LEN==24&&sBUF[24]==0x00)//���֡ͷ֡β�ͳ������� ����
//		{
//		  RC_LEN=0;//���ճ������� �ȴ��´θ�ֵ
//			//�������� �����ټ�
//		  for(i=0;i<25;i++)
//			{
//				if(i==0) 
//				{
//					uart2RxFlag=0;//��־λ����
//				}
//			  Data[i]=0;//�ȸ��������������� �ŷ���������¸�ֵ
//			}
//			//���ݴ������ֵת�Ƶ�����������
//			for(i=0;i<25;i++)
//			{
//			  Data[i]=sBUF[i];
//			}
//			uart2RxFlag=1;//��������ȷ������
//			contrl_flag=Data[23];//��־λ
//}
//}

//����ú����յ�����Ϊdata_ch[j]
//�����յ���ͨ������
//ѭ���ö��˵��´������
void Cal_RcData(void)
{
	u8 i,j=0;
	//ժȡ22λͨ������ 
	for(i=1;i<23;i++)//ȥ��֡ͷ ֡β 
	{
		data_22[j++]=Data[i];
	}	
	//ת��λ2���ƴ洢
	for(i=0;i<22;i++)
	{
		int k=1;
		for(j=0;j<8;j++)
		{	 
			data_b[j+8*i]=(data_22[i]&k)>>j;
			k<<=1;	
		} 	
	}
	  memset(data_ch,0, sizeof data_ch); //��������
	//ת��ΪSBUSҪ��ĸ�ʽ 6��ͨ�� 16
	for(j=0;j<6;j++)
	{
		for(i=10;i>0;i--)
		{
			data_ch[j]=data_ch[j]|(data_b[i+j*11]<<i);
		}     
	}	
}





