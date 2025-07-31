#include "can.h"
#include "led.h"
#include "delay.h"
#include "usart.h"
#include "stm32f10x_can.h"

int  CAN_RX_STA=0;//���ձ�־λ
int  Can_TX_Buff[16] = {0}; //���ͻ��� �з�����
int  Can_RX_Buff[16] = {0};//���ջ��� 

int  Shifting_Buff[16]={0};//���ٻ���


//�Ӽ���
double jerk = 0.1;   // ��ֵ jerk
int num_steps = 100; // ��������
int acceleration=10;//���ٶ�


//tsjw:����ͬ����Ծʱ�䵥Ԫ.��Χ:CAN_SJW_1tq~ CAN_SJW_4tq
//tbs2:ʱ���2��ʱ�䵥Ԫ.   ��Χ:CAN_BS2_1tq~CAN_BS2_8tq;
//tbs1:ʱ���1��ʱ�䵥Ԫ.   ��Χ:CAN_BS1_1tq ~CAN_BS1_16tq
//brp :�����ʷ�Ƶ��.��Χ:1~1024;  tq=(brp)*tpclk1
//������=Fpclk1/((tbs1+1+tbs2+1+1)*brp);
//Fpclk1��ʱ���ڳ�ʼ����ʱ������Ϊ36M,�������CAN_Mode_Init(CAN_SJW_1tq,CAN_BS2_8tq,CAN_BS1_9tq,4,CAN_Mode_LoopBack);
//������Ϊ:36M/((8+9+1)*4)=500Kbps
//mode:CAN_Mode_Normal,��ͨģʽ;CAN_Mode_LoopBack,�ػ�ģʽ;

//CAN��ʼ��
//����ֵ:0,��ʼ��OK;
//����,��ʼ��ʧ��; 
//Ӳ���ӿ� PB8 CANRX PB9 CANTX

void CAN_Mode_Init(void)
{
		GPIO_InitTypeDef GPIO_InitStructure; //IO
		CAN_InitTypeDef  CAN_InitStructure;  //CAM����
		CAN_FilterInitTypeDef  CAN_FilterInitStructure;//������
		NVIC_InitTypeDef  NVIC_InitStructure;//��һ��CAN�ж� �������ȼ�

		RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO|RCC_APB2Periph_GPIOB,ENABLE);//ʹ��PORTBʱ��	                   											 
  	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);//ʹ��CAN1ʱ��
   
	//#define GPIO_Remap_CAN    GPIO_Remap1_CAN1 û���õ���ӳ��I/O
		GPIO_PinRemapConfig(GPIO_Remap1_CAN1,ENABLE);//ʹ����ӳ��
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//��������
    GPIO_Init(GPIOB, &GPIO_InitStructure);		//��ʼ��IO
   
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//��������
    GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��IO 
	  
 	// CAN RX interrupt �ж����ȼ�
		NVIC_InitStructure.NVIC_IRQChannel=USB_LP_CAN1_RX0_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);  
	
		//CAN��Ԫ����
		/* CAN register init */
		CAN_DeInit(CAN1);//������CAN��ȫ���Ĵ�������Ϊȱʡֵ
		CAN_StructInit(&CAN_InitStructure);//��CAN_InitStruct�е�ÿһ��������ȱʡֵ����

		/* CAN cell init */
		CAN_InitStructure.CAN_TTCM=DISABLE;  //��ֹʱ�䴥��ͨ��ģʽ
		CAN_InitStructure.CAN_ABOM=DISABLE;  //�����CAN_MCR�Ĵ�����INRQλ������1�����0��һ��Ӳ�����
																				 //��128��11λ����������λ�����˳�����״̬��
		CAN_InitStructure.CAN_AWUM=DISABLE;  //˯��ģʽͨ�����CAN_MCR�Ĵ�����SLEEPλ�����������
		CAN_InitStructure.CAN_NART=DISABLE;   //CAN����ֻ������1�Σ����ܷ��͵Ľ����Σ��ɹ���������ٲö�ʧ��
		CAN_InitStructure.CAN_RFLM=DISABLE;  //�ڽ������ʱFIFOδ��������������FIFO�ı���δ����������һ���յ��ı��ĻḲ��ԭ�еı���
		CAN_InitStructure.CAN_TXFP=DISABLE;//û��ʹ�ܷ���FIFO���ȼ�
		CAN_InitStructure.CAN_Mode=CAN_Mode_Normal;//CANӲ������������ģʽ
		
    //������Ϊ��72M/2/60(1+3+2)=0.1 ��100K
		CAN_InitStructure.CAN_SJW=CAN_SJW_1tq; //����ͬ����Ծ���1��ʱ�䵥λ
		CAN_InitStructure.CAN_BS1=CAN_BS1_3tq; //ʱ���1Ϊ3��ʱ�䵥λ
		CAN_InitStructure.CAN_BS2=CAN_BS2_2tq; //ʱ���2Ϊ2��ʱ�䵥λ
		CAN_InitStructure.CAN_Prescaler=60;  //ʱ�䵥λ����Ϊ60	

//    //������Ϊ��72M/2/9/(1+5+2)=0.5 ��500K                               
//		CAN_InitStructure.CAN_SJW=CAN_SJW_1tq; //����ͬ����Ծ���1��ʱ�䵥λ
//		CAN_InitStructure.CAN_BS1=CAN_BS1_5tq; //ʱ���1Ϊ5��ʱ�䵥λ
//		CAN_InitStructure.CAN_BS2=CAN_BS2_2tq; //ʱ���2Ϊ2��ʱ�䵥λ
//		CAN_InitStructure.CAN_Prescaler=9;  //ʱ�䵥λ����Ϊ9 	
		CAN_Init(CAN1,&CAN_InitStructure);

	 
		/* CAN filter init */
		CAN_FilterInitStructure.CAN_FilterNumber=1;//ָ��������Ϊ1
		CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdMask;//ָ��������Ϊ��ʶ������λģʽ
		CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit;//������λ��Ϊ32λ
		CAN_FilterInitStructure.CAN_FilterIdHigh=0x0000;// ��������ʶ���ĸ�16λֵ
		CAN_FilterInitStructure.CAN_FilterIdLow=0x0000;//	 ��������ʶ���ĵ�16λֵ
		CAN_FilterInitStructure.CAN_FilterMaskIdHigh=0x0000;//���������α�ʶ���ĸ�16λֵ
		CAN_FilterInitStructure.CAN_FilterMaskIdLow=0x0000;//	���������α�ʶ���ĵ�16λֵ
		CAN_FilterInitStructure.CAN_FilterFIFOAssignment=CAN_FIFO0;// �趨��ָ���������FIFOΪ0
		CAN_FilterInitStructure.CAN_FilterActivation=ENABLE;// ʹ�ܹ�����
		CAN_FilterInit(&CAN_FilterInitStructure);//	������Ĳ�����ʼ��������

		/* CAN FIFO0 message pending interrupt enable */ 
	  CAN_ITConfig(CAN1,CAN_IT_FMP0, ENABLE); //ʹ��FIFO0��Ϣ�Һ��ж�
}   
 
//���͸�ʽ --- CAN_ID CAN_LEN CAN_DATA
//�жϷ�����	����CAN����		    
void USB_LP_CAN1_RX0_IRQHandler(void)
{
		int i=0;
  	CanRxMsg RxMessage;//�������սṹ�����  
    CAN_Receive(CAN1,0,&RxMessage);//����CAN1������
	//�ѽṹ������ݱ��浽����������
	  Can_RX_Buff[0]=RxMessage.StdId;//id
	  Can_RX_Buff[1]=RxMessage.DLC;//���ݳ���
	  for(i=0;i<Can_RX_Buff[1];i++)
	  {	 
			Can_RX_Buff[i+2]=RxMessage.Data[i];
	  }
//		for(i=0;i<Can_RX_Buff[1]+2;i++)
//		{
//			printf("%X",Can_RX_Buff[i]);
//			if(i==Can_RX_Buff[1]+1)
//				printf("\r\n");
//		}
		 CAN_RX_STA=1;
}

/* 
����ת������  ID��С��10
DATA[0]��0x02 ����Ϊ�ٶȿ��ƣ�0x0A ����Ϊ���ü��ٶȣ�0x10 ����Ϊ���ü��ٶȡ�
DATA[1]�������ٶ�Ϊ int �ͣ�4 ���ֽڡ��ٶ�ֵ�� 24 λ��
DATA[2]�������ٶ�Ϊ int �ͣ�4 ���ֽڡ��ٶ�ֵ�� 16 λ��
DATA[3]�������ٶ�Ϊ int �ͣ�4 ���ֽڡ��ٶ�ֵ�� 8 λ��
DATA[4]�������ٶ�Ϊ int �ͣ�4 ���ֽڡ��ٶ�ֵ�� 8 λ
*/
void can_tx_speed(u8 can_id,char Data1,char Data2,char Data3,char Data4)
{ 
  CanTxMsg TxMessage;  

  TxMessage.StdId=can_id;	//��׼��ʶ��Ϊ0x00
  TxMessage.ExtId=can_id; //��չ��ʶ��0x0000
  TxMessage.IDE=CAN_Id_Standard;//ʹ�ñ�׼��ʶ��
  TxMessage.RTR=CAN_RTR_DATA;//Ϊ����֡
  TxMessage.DLC=5;	//	��Ϣ�����ݳ���Ϊ2���ֽ�
	
  TxMessage.Data[0]=0x02; //��һ���ֽ����� ��ʾΪ�ٶȿ���
  TxMessage.Data[1]=Data1; //�ڶ����ֽ�����
  TxMessage.Data[2]=Data2; //�ڶ����ֽ����� 	
	TxMessage.Data[3]=Data3; //�ڶ����ֽ�����
  TxMessage.Data[4]=Data4; //�ڶ����ֽ����� 	
  CAN_Transmit(CAN1,&TxMessage); //��������
}

/*
�Ӽ����㷨���˼· 
1.����Э���ȡ��ǰ���ռ�ձ�
2.��Ŀ��ֵ�Ƚϣ��趨�ӡ�����ģʽ
3.�㶨���ٶ� ���мӼ��ٲ���
*/
//��������ϱ���ռ�ձ���Ϣ
// ���μӼ����㷨
double trapezoidal_speed(double target_speed, double current_speed) 
	{
		int i=0;
    for(int i=0;i<num_steps;i++)
		{
        current_speed+=(acceleration+jerk*(current_speed-target_speed))/(1+jerk*i);
        printf("Step%d: Speed = %f\n",i,current_speed);
    }
    return current_speed;
 }


//can����һ������(�̶���ʽ:IDΪ0X12,��׼֡,����֡)	
//len:���ݳ���(���Ϊ8)				     
//msg:����ָ��,���Ϊ8���ֽ�.
//����ֵ:0,�ɹ�;
//����,ʧ��;
void Can_Send_Msg(char* msg,u8 len,uint32_t CAN_ID)
{	
  u8 mbox;
  u16 i=0;
	//CAN Tx message structure definition 
  CanTxMsg TxMessage;//�ṹ�嶨��
  TxMessage.StdId=CAN_ID;					 // ��׼��ʶ��Ϊ0
  TxMessage.ExtId=CAN_ID;				   // ������չ��ʾ����29λ��
  TxMessage.IDE=CAN_Id_Standard;			           // ʹ�ñ�׼֡
  TxMessage.RTR=CAN_RTR_Data;		             // ��Ϣ����Ϊ����֡��һ֡8λ
  TxMessage.DLC=len;							 // ������Ϣ����
	
  for(i=0;i<len;i++)               //Ҫ���͵�����
		{
			 TxMessage.Data[i]=msg[i];	 // ��һ֡��Ϣ  
		}		
		mbox=CAN_Transmit(CAN1, &TxMessage);  //��������� 
		i=0;
		while((CAN_TransmitStatus(CAN1,mbox)==CAN_TxStatus_Failed)&&(i<0XFFF))
	  i++;	//�ȴ����ͽ���	
}


/* ���������ֽڵ�����*/
void can_tx(u8 Data1,u8 Data2)
{ 
  CanTxMsg TxMessage;  

  TxMessage.StdId=0x00;	//��׼��ʶ��Ϊ0x00
  TxMessage.ExtId=0x0000; //��չ��ʶ��0x0000
  TxMessage.IDE=CAN_Id_Standard;//ʹ�ñ�׼��ʶ��
  TxMessage.RTR=CAN_RTR_DATA;//Ϊ����֡
  TxMessage.DLC=2;	//	��Ϣ�����ݳ���Ϊ2���ֽ�
  TxMessage.Data[0]=Data1; //��һ���ֽ�����
  TxMessage.Data[1]=Data2; //�ڶ����ֽ����� 
  CAN_Transmit(CAN1,&TxMessage); //��������
}

/* ��������֡������*/
void can_heartbeat_tx(u8 can_id)
{ 
  CanTxMsg TxMessage;  
	//id
  TxMessage.StdId=can_id;	//��׼��ʶ��Ϊ0x00
  TxMessage.ExtId=can_id; //��չ��ʶ��0x0000
	//��׼֡ ����֡
  TxMessage.IDE=CAN_Id_Standard;//ʹ�ñ�׼��ʶ��
  TxMessage.RTR=CAN_RTR_DATA;//Ϊ����֡
	//֡����
  TxMessage.DLC=1;	//	��Ϣ�����ݳ���Ϊ2���ֽ�
	//������Ϣ
  TxMessage.Data[0]=00; //��һ���ֽ�����
  CAN_Transmit(CAN1,&TxMessage); //��������
}




/* ��CAN�����ַ����� ID��С��10*/
void can_tx_inf_usart(u8 can_id,char* Data)
{ 
	u8 i;
  CanTxMsg TxMessage;  
  TxMessage.StdId=can_id;	//��׼��ʶ��Ϊ0x00
  TxMessage.ExtId=can_id; //��չ��ʶ��0x0000
  TxMessage.IDE=CAN_Id_Standard;//ʹ�ñ�׼��ʶ��
  TxMessage.RTR=CAN_RTR_DATA;//Ϊ����֡
  TxMessage.DLC=sizeof(Data);	//	��Ϣ�����ݳ���Ϊ2���ֽ�
	for(i=0;i<sizeof(Data);i++)
	{
		TxMessage.Data[i]=Data[i]; //��һ���ֽ�����
	} 
  CAN_Transmit(CAN1,&TxMessage); //��������
}

/* ����ռ�ձ�����  ID��С��10*/
void can_tx_duty_usart(char can_id,char Data1,char Data2)
{ 
  CanTxMsg TxMessage;  

  TxMessage.StdId=can_id;	//��׼��ʶ��Ϊ0x00
  TxMessage.ExtId=can_id; //��չ��ʶ��0x0000
  TxMessage.IDE=CAN_Id_Standard;//ʹ�ñ�׼��ʶ��
  TxMessage.RTR=CAN_RTR_DATA;//Ϊ����֡
  TxMessage.DLC=3;	//	��Ϣ�����ݳ���Ϊ2���ֽ�
	
  TxMessage.Data[0]=Data1; //��һ���ֽ�����
  TxMessage.Data[1]=Data2; //�ڶ����ֽ����� 
  CAN_Transmit(CAN1,&TxMessage); //��������
}

//����16���Ƶ��ַ�ƴ��
int hexcharToInt(char c)
{
	if (c>='0'&&c<='9') 
		return (c-'0');
	if (c >='A'&& c<='F') 
		return (c-'A'+10);
	if (c>='a'&&c<='f') 
		return (c-'a'+10);
	return 0;
}










