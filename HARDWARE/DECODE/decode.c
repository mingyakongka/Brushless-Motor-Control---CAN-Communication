#include "decode.h"
#include "can.h"
#include "delay.h"
#include "usart.h"
#include "stm32f10x_can.h"
#include "sbus.h"

volatile MFCCtrlByte0Def  _MFCCtrlByte0;
volatile MFCMonitorStusDef  _MFCMonitorStus;

volatile u8  ThrottleReq;//精度0.4%
volatile u8  BrakeReq;//精度0.4%
volatile int8_t  SteerReq;//分辨率1，偏移量-100
volatile int8_t  ManualSteerReq;//分辨率1，偏移量-100

volatile u16  LeftSpeed;//unit:rpm
volatile u16  RightSpeed;//unit:rpm
volatile u16  EngSpeed;//unit:rpm
u8 MFC2BCUbuf[8];


u8 Testbuf[8];

volatile u8  MFC_ErrorCode;
//待测数据
/************************************************************************************************
*通道0  右侧遥杆(挨着C),  向左打死292，			向右打死1678
*
*通道1	右侧遥杆(挨着C),	向左打死540，			向后打死1294
*
*通道2	左侧遥杆(挨着B),	向前打死236,		 	向后打死1622
*
*通道3  左侧遥杆(挨着B),	向左打死330,		 	向右打死1716
*
*通道4	拨动按钮(就是F),	向下拨306，				向上拨1694
*
*通道5	左侧遥杆(挨着B),	向前打死306，			向后打死1694 这个通道的数据跟通道2的数据变化是一样的，冗余了，取一个值应该就可以了
*
*通道6	拨动按钮(就是C),	向前拨306，				拨中间1000，				向后拨1694

****************************************************************************/

void SBUS2MFCdecode(void)
{
		if(uart2RxFlag==1)//串口2收到数据
			{
			uart2RxFlag=0;//标志位清零
			Cal_RcData();//解析sbus数据
		
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
				LightCtrl=1;//近光=1
			if(data_ch[LightCtrlChannel-1]<400) 
				LightCtrl=2;//远光=2
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


//把解析的SBUS信号 变为相应的控制信号 通过CAN发给驱动器
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



