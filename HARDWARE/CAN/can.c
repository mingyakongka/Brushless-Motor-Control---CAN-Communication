#include "can.h"
#include "led.h"
#include "delay.h"
#include "usart.h"
#include "stm32f10x_can.h"

int  CAN_RX_STA=0;//接收标志位
int  Can_TX_Buff[16] = {0}; //发送缓存 有符号数
int  Can_RX_Buff[16] = {0};//接收缓存 

int  Shifting_Buff[16]={0};//变速缓存


//加减速
double jerk = 0.1;   // 常值 jerk
int num_steps = 100; // 步进次数
int acceleration=10;//加速度


//tsjw:重新同步跳跃时间单元.范围:CAN_SJW_1tq~ CAN_SJW_4tq
//tbs2:时间段2的时间单元.   范围:CAN_BS2_1tq~CAN_BS2_8tq;
//tbs1:时间段1的时间单元.   范围:CAN_BS1_1tq ~CAN_BS1_16tq
//brp :波特率分频器.范围:1~1024;  tq=(brp)*tpclk1
//波特率=Fpclk1/((tbs1+1+tbs2+1+1)*brp);
//Fpclk1的时钟在初始化的时候设置为36M,如果设置CAN_Mode_Init(CAN_SJW_1tq,CAN_BS2_8tq,CAN_BS1_9tq,4,CAN_Mode_LoopBack);
//则波特率为:36M/((8+9+1)*4)=500Kbps
//mode:CAN_Mode_Normal,普通模式;CAN_Mode_LoopBack,回环模式;

//CAN初始化
//返回值:0,初始化OK;
//其他,初始化失败; 
//硬件接口 PB8 CANRX PB9 CANTX

void CAN_Mode_Init(void)
{
		GPIO_InitTypeDef GPIO_InitStructure; //IO
		CAN_InitTypeDef  CAN_InitStructure;  //CAM配置
		CAN_FilterInitTypeDef  CAN_FilterInitStructure;//过滤器
		NVIC_InitTypeDef  NVIC_InitStructure;//有一个CAN中断 配置优先级

		RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO|RCC_APB2Periph_GPIOB,ENABLE);//使能PORTB时钟	                   											 
  	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);//使能CAN1时钟
   
	//#define GPIO_Remap_CAN    GPIO_Remap1_CAN1 没有用到重映射I/O
		GPIO_PinRemapConfig(GPIO_Remap1_CAN1,ENABLE);//使能重映射
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽
    GPIO_Init(GPIOB, &GPIO_InitStructure);		//初始化IO
   
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//上拉输入
    GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化IO 
	  
 	// CAN RX interrupt 中断优先级
		NVIC_InitStructure.NVIC_IRQChannel=USB_LP_CAN1_RX0_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);  
	
		//CAN单元设置
		/* CAN register init */
		CAN_DeInit(CAN1);//将外设CAN的全部寄存器重设为缺省值
		CAN_StructInit(&CAN_InitStructure);//把CAN_InitStruct中的每一个参数按缺省值填入

		/* CAN cell init */
		CAN_InitStructure.CAN_TTCM=DISABLE;  //禁止时间触发通信模式
		CAN_InitStructure.CAN_ABOM=DISABLE;  //软件对CAN_MCR寄存器的INRQ位进行置1随后清0后，一旦硬件检测
																				 //到128次11位连续的隐性位，就退出离线状态。
		CAN_InitStructure.CAN_AWUM=DISABLE;  //睡眠模式通过清除CAN_MCR寄存器的SLEEP位，由软件唤醒
		CAN_InitStructure.CAN_NART=DISABLE;   //CAN报文只被发送1次，不管发送的结果如何（成功、出错或仲裁丢失）
		CAN_InitStructure.CAN_RFLM=DISABLE;  //在接收溢出时FIFO未被锁定，当接收FIFO的报文未被读出，下一个收到的报文会覆盖原有的报文
		CAN_InitStructure.CAN_TXFP=DISABLE;//没有使能发送FIFO优先级
		CAN_InitStructure.CAN_Mode=CAN_Mode_Normal;//CAN硬件工作在正常模式
		
    //波特率为：72M/2/60(1+3+2)=0.1 即100K
		CAN_InitStructure.CAN_SJW=CAN_SJW_1tq; //重新同步跳跃宽度1个时间单位
		CAN_InitStructure.CAN_BS1=CAN_BS1_3tq; //时间段1为3个时间单位
		CAN_InitStructure.CAN_BS2=CAN_BS2_2tq; //时间段2为2个时间单位
		CAN_InitStructure.CAN_Prescaler=60;  //时间单位长度为60	

//    //波特率为：72M/2/9/(1+5+2)=0.5 即500K                               
//		CAN_InitStructure.CAN_SJW=CAN_SJW_1tq; //重新同步跳跃宽度1个时间单位
//		CAN_InitStructure.CAN_BS1=CAN_BS1_5tq; //时间段1为5个时间单位
//		CAN_InitStructure.CAN_BS2=CAN_BS2_2tq; //时间段2为2个时间单位
//		CAN_InitStructure.CAN_Prescaler=9;  //时间单位长度为9 	
		CAN_Init(CAN1,&CAN_InitStructure);

	 
		/* CAN filter init */
		CAN_FilterInitStructure.CAN_FilterNumber=1;//指定过滤器为1
		CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdMask;//指定过滤器为标识符屏蔽位模式
		CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit;//过滤器位宽为32位
		CAN_FilterInitStructure.CAN_FilterIdHigh=0x0000;// 过滤器标识符的高16位值
		CAN_FilterInitStructure.CAN_FilterIdLow=0x0000;//	 过滤器标识符的低16位值
		CAN_FilterInitStructure.CAN_FilterMaskIdHigh=0x0000;//过滤器屏蔽标识符的高16位值
		CAN_FilterInitStructure.CAN_FilterMaskIdLow=0x0000;//	过滤器屏蔽标识符的低16位值
		CAN_FilterInitStructure.CAN_FilterFIFOAssignment=CAN_FIFO0;// 设定了指向过滤器的FIFO为0
		CAN_FilterInitStructure.CAN_FilterActivation=ENABLE;// 使能过滤器
		CAN_FilterInit(&CAN_FilterInitStructure);//	按上面的参数初始化过滤器

		/* CAN FIFO0 message pending interrupt enable */ 
	  CAN_ITConfig(CAN1,CAN_IT_FMP0, ENABLE); //使能FIFO0消息挂号中断
}   
 
//发送格式 --- CAN_ID CAN_LEN CAN_DATA
//中断服务函数	接收CAN数据		    
void USB_LP_CAN1_RX0_IRQHandler(void)
{
		int i=0;
  	CanRxMsg RxMessage;//命名接收结构体变量  
    CAN_Receive(CAN1,0,&RxMessage);//接受CAN1的数据
	//把结构体的数据保存到缓存数组里
	  Can_RX_Buff[0]=RxMessage.StdId;//id
	  Can_RX_Buff[1]=RxMessage.DLC;//数据长度
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
发送转速数据  ID号小于10
DATA[0]：0x02 代表为速度控制，0x0A 代表为设置加速度，0x10 代表为设置减速度。
DATA[1]：控制速度为 int 型，4 个字节。速度值高 24 位。
DATA[2]：控制速度为 int 型，4 个字节。速度值高 16 位。
DATA[3]：控制速度为 int 型，4 个字节。速度值高 8 位。
DATA[4]：控制速度为 int 型，4 个字节。速度值高 8 位
*/
void can_tx_speed(u8 can_id,char Data1,char Data2,char Data3,char Data4)
{ 
  CanTxMsg TxMessage;  

  TxMessage.StdId=can_id;	//标准标识符为0x00
  TxMessage.ExtId=can_id; //扩展标识符0x0000
  TxMessage.IDE=CAN_Id_Standard;//使用标准标识符
  TxMessage.RTR=CAN_RTR_DATA;//为数据帧
  TxMessage.DLC=5;	//	消息的数据长度为2个字节
	
  TxMessage.Data[0]=0x02; //第一个字节数据 表示为速度控制
  TxMessage.Data[1]=Data1; //第二个字节数据
  TxMessage.Data[2]=Data2; //第二个字节数据 	
	TxMessage.Data[3]=Data3; //第二个字节数据
  TxMessage.Data[4]=Data4; //第二个字节数据 	
  CAN_Transmit(CAN1,&TxMessage); //发送数据
}

/*
加减速算法设计思路 
1.解析协议获取当前电机占空比
2.与目标值比较，设定加、减速模式
3.恒定加速度 进行加减速操作
*/
//解析电机上报的占空比信息
// 梯形加减速算法
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


//can发送一组数据(固定格式:ID为0X12,标准帧,数据帧)	
//len:数据长度(最大为8)				     
//msg:数据指针,最大为8个字节.
//返回值:0,成功;
//其他,失败;
void Can_Send_Msg(char* msg,u8 len,uint32_t CAN_ID)
{	
  u8 mbox;
  u16 i=0;
	//CAN Tx message structure definition 
  CanTxMsg TxMessage;//结构体定义
  TxMessage.StdId=CAN_ID;					 // 标准标识符为0
  TxMessage.ExtId=CAN_ID;				   // 设置扩展标示符（29位）
  TxMessage.IDE=CAN_Id_Standard;			           // 使用标准帧
  TxMessage.RTR=CAN_RTR_Data;		             // 消息类型为数据帧，一帧8位
  TxMessage.DLC=len;							 // 发送信息长度
	
  for(i=0;i<len;i++)               //要发送的数据
		{
			 TxMessage.Data[i]=msg[i];	 // 第一帧信息  
		}		
		mbox=CAN_Transmit(CAN1, &TxMessage);  //返回邮箱号 
		i=0;
		while((CAN_TransmitStatus(CAN1,mbox)==CAN_TxStatus_Failed)&&(i<0XFFF))
	  i++;	//等待发送结束	
}


/* 发送两个字节的数据*/
void can_tx(u8 Data1,u8 Data2)
{ 
  CanTxMsg TxMessage;  

  TxMessage.StdId=0x00;	//标准标识符为0x00
  TxMessage.ExtId=0x0000; //扩展标识符0x0000
  TxMessage.IDE=CAN_Id_Standard;//使用标准标识符
  TxMessage.RTR=CAN_RTR_DATA;//为数据帧
  TxMessage.DLC=2;	//	消息的数据长度为2个字节
  TxMessage.Data[0]=Data1; //第一个字节数据
  TxMessage.Data[1]=Data2; //第二个字节数据 
  CAN_Transmit(CAN1,&TxMessage); //发送数据
}

/* 发送心跳帧的数据*/
void can_heartbeat_tx(u8 can_id)
{ 
  CanTxMsg TxMessage;  
	//id
  TxMessage.StdId=can_id;	//标准标识符为0x00
  TxMessage.ExtId=can_id; //扩展标识符0x0000
	//标准帧 数据帧
  TxMessage.IDE=CAN_Id_Standard;//使用标准标识符
  TxMessage.RTR=CAN_RTR_DATA;//为数据帧
	//帧长度
  TxMessage.DLC=1;	//	消息的数据长度为2个字节
	//心跳信息
  TxMessage.Data[0]=00; //第一个字节数据
  CAN_Transmit(CAN1,&TxMessage); //发送数据
}




/* 向CAN发送字符数组 ID号小于10*/
void can_tx_inf_usart(u8 can_id,char* Data)
{ 
	u8 i;
  CanTxMsg TxMessage;  
  TxMessage.StdId=can_id;	//标准标识符为0x00
  TxMessage.ExtId=can_id; //扩展标识符0x0000
  TxMessage.IDE=CAN_Id_Standard;//使用标准标识符
  TxMessage.RTR=CAN_RTR_DATA;//为数据帧
  TxMessage.DLC=sizeof(Data);	//	消息的数据长度为2个字节
	for(i=0;i<sizeof(Data);i++)
	{
		TxMessage.Data[i]=Data[i]; //第一个字节数据
	} 
  CAN_Transmit(CAN1,&TxMessage); //发送数据
}

/* 发送占空比数据  ID号小于10*/
void can_tx_duty_usart(char can_id,char Data1,char Data2)
{ 
  CanTxMsg TxMessage;  

  TxMessage.StdId=can_id;	//标准标识符为0x00
  TxMessage.ExtId=can_id; //扩展标识符0x0000
  TxMessage.IDE=CAN_Id_Standard;//使用标准标识符
  TxMessage.RTR=CAN_RTR_DATA;//为数据帧
  TxMessage.DLC=3;	//	消息的数据长度为2个字节
	
  TxMessage.Data[0]=Data1; //第一个字节数据
  TxMessage.Data[1]=Data2; //第二个字节数据 
  CAN_Transmit(CAN1,&TxMessage); //发送数据
}

//两个16进制的字符拼接
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










