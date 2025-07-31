#include "decode.h"
#include "can.h"
#include "delay.h"
#include "usart.h"
#include "stm32f10x_can.h"
#include "sbus.h"

volatile MFCCtrlByte0Def  _MFCCtrlByte0;
volatile MFCMonitorStusDef  _MFCMonitorStus;

volatile u8  ThrottleReq;//����0.4%
volatile u8  BrakeReq;//����0.4%
volatile int8_t  SteerReq;//�ֱ���1��ƫ����-100
volatile int8_t  ManualSteerReq;//�ֱ���1��ƫ����-100

volatile u16  LeftSpeed;//unit:rpm
volatile u16  RightSpeed;//unit:rpm
volatile u16  EngSpeed;//unit:rpm
u8 MFC2BCUbuf[8];


u8 Testbuf[8];

volatile u8  MFC_ErrorCode;
//��������
/************************************************************************************************
*ͨ��0  �Ҳ�ң��(����C),  �������292��			���Ҵ���1678
*
*ͨ��1	�Ҳ�ң��(����C),	�������540��			������1294
*
*ͨ��2	���ң��(����B),	��ǰ����236,		 	������1622
*
*ͨ��3  ���ң��(����B),	�������330,		 	���Ҵ���1716
*
*ͨ��4	������ť(����F),	���²�306��				���ϲ�1694
*
*ͨ��5	���ң��(����B),	��ǰ����306��			������1694 ���ͨ�������ݸ�ͨ��2�����ݱ仯��һ���ģ������ˣ�ȡһ��ֵӦ�þͿ�����
*
*ͨ��6	������ť(����C),	��ǰ��306��				���м�1000��				���1694

****************************************************************************/

void SBUS2MFCdecode(void)
{
		if(uart2RxFlag==1)//����2�յ�����
			{
			uart2RxFlag=0;//��־λ����
			Cal_RcData();//����sbus����
		
			if(data_ch[GearReqChannel-1]<400) 
			{
				GearReq=1;
			}
      else if(data_ch[GearReqChannel-1]>1400) 
			{
				GearReq=2;
			}
      else 
				GearReq=0;
				
			if(data_ch[HandBrakeCtrlChannel-1]<400) 
				HandBrakeCtrl=1;
      else 
				HandBrakeCtrl=0;
				
			if(data_ch[LightCtrlChannel-1]>1400) 
				LightCtrl=1;//����=1
			if(data_ch[LightCtrlChannel-1]<400) 
				LightCtrl=2;//Զ��=2
      else 
				LightCtrl=0;
				
			if(data_ch[Ignit_FlameoutCtrlChannel-1]>1400) 
				IgnitCtrl=1;
      else 
				IgnitCtrl=0;

			if(data_ch[Ignit_FlameoutCtrlChannel-1]<400) 
				FlameoutCtrl=1;
      else 
				FlameoutCtrl=0;	

			if(data_ch[TrumpetCtrlChannel-1]<400) 
				TrumpetCtrl=1;
      else 
				TrumpetCtrl=0;	
			
      if(data_ch[Throttle_BrakeReqChannel-1]>=1024) 
			{
				ThrottleReq=(data_ch[Throttle_BrakeReqChannel-1]-1024)*250/(1680-1024);
				//ThrottleReq=(u32)(data_ch[Throttle_BrakeReqChannel-1]-1000)*250/800;	
				BrakeReq=0;
			}
      else
			{
				BrakeReq=(1024-data_ch[Throttle_BrakeReqChannel-1])*250/(1024-368);
				//BrakeReq=(u32)(1000-data_ch[Throttle_BrakeReqChannel-1])/800*250;
				ThrottleReq=0;	
			}
				SteerReq=(data_ch[SteerReqChannel-1]-1024)*200/(1680-368); 
		}			
}

//void ManualSteerReqScan(void)
//{

//		
//}


//�ѽ�����SBUS�ź� ��Ϊ��Ӧ�Ŀ����ź� ͨ��CAN����������
void MFC_FeedBack(void)
{
	MFC2BCUbuf[0]=MFCCtrlByte0;
	MFC2BCUbuf[1]=ThrottleReq;
	MFC2BCUbuf[2]=BrakeReq;
	MFC2BCUbuf[3]=SteerReq+100;
	MFC2BCUbuf[4]=(LeftSpeed+5000)&0xFF;
	MFC2BCUbuf[5]=(LeftSpeed+5000)>>8;
	MFC2BCUbuf[6]=(RightSpeed+5000)&0xFF;
	MFC2BCUbuf[7]=(RightSpeed+5000)>>8;
	Can_Send_Msg(	MFC2BCUbuf,8,0x99);
}



