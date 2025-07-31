#include "sys.h"
#include "usart.h"	  
#include "can.h"
#include "string.h"


//串口1
//支持适应不同频率下的串口波特率设置.
//加入了对printf的支持
//增加了串口接收命令功能.
//修正了printf第一个字符丢失的bug

//V1.4修改说明
//1,修改串口初始化IO的bug
//2,修改了USART_RX_STA,使得串口最大接收字节数为2的14次方
//3,增加了USART_REC_LEN,用于定义串口最大允许接收的字节数(不大于2的14次方)
//4,修改了EN_USART1_RX的使能方式

//V1.5修改说明
//1,增加了对UCOSII的支持  

//////////////////////////////////////////////////////////////////
//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  

#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
int _sys_exit(int x) 
{ 
	x = x; 
} 

//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
    USART1->DR = (u8) ch;      
	return ch;
}

//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	
int  USART_RX_BUF[16]={0};     //接收缓冲,最大USART_REC_LEN个字节.
int  USART_TX_BUF[16]={0};     //接收缓冲,最大USART_REC_LEN个字节.
int  data[16]={0};//实际数据存储

extern int Can_TX_Buff[16];
u8 count=0;
u8 USART_RX_STA=0;       //接收状态标记	  
  
void uart_init(u32 bound)
	{
  //GPIO端口设置
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//使能USART1，GPIOA时钟
  
	//USART1_TX   GPIOA.9
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.9
   
  //USART1_RX	  GPIOA.10初始化
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.10  

  //Usart1 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;//选择中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
  
   //USART 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

  USART_Init(USART1, &USART_InitStructure); //初始化串口1
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启串口接受中断
  USART_Cmd(USART1, ENABLE);                    //使能串口1 

}

//使用自定义协议接收十六进制数据
//====================================串口中断服务程序=================================================

/*--------------------------------接收协议-----------------------------------
//---------------- 5a CAN_ID DATA_len 00 00 00 00 00 5b ----------------------
//数据头0x5a + CAN_ID + 数据 + 数据尾0d5b 定长 九个字节
--------------------------------------------------------------------------*/
//中断函数 中断：一个字节(8位)的接收每收到一个字节产生一次中断
//DMA:可配置为一帧一帧的接收，一帧数据接收完了再产生中断
void USART1_IRQHandler(void) 
{
  u8 i;
	if(USART_GetITStatus(USART1,USART_IT_RXNE)!= RESET)//串口接收到数据
	{
		USART_ClearITPendingBit(USART1,USART_IT_RXNE);//首先清除中断标志位
		//返回最新收到的数据
	  USART_RX_BUF[count++]=USART_ReceiveData(USART1);//把串口2的数据给到SBUS
		
		if(USART_RX_BUF[0]==0x5a&&count==9&&USART_RX_BUF[8]==0x5b)//如果帧头帧尾和长度满足 传输
		{
		  count=0;//接收长度置零 等待下次赋值
			//清零数组 可以再简化
			memset(data,0,sizeof(data)); //数组清零
			//把暂存数组的值转移到数据数组里
			for(i=0;i<9;i++)
			{
			  data[i]=USART_RX_BUF[i];//接收的数据保存到data
				printf("第%d=%X\r\n",i,data[i]);
			}
			USART_RX_STA=1;//接收完正确的数组
		}
	}
}

/*--------------------------------接收协议-----------------------------------
//---------------- 5a CAN_ID DATA_len 00 00 00 00 00 5b ----------------------
//数据头0x5a + CAN_ID + 数据长度 +0X02转速控制 数据尾0d5b 定长 九个字节
--------------------------------------------------------------------------*/
//void send_cmd(u16* data)//将串口数据转发个CAN
//{
//	u8 i=0;
//	CanTxMsg TxMessage;  
//  TxMessage.StdId=data[1];	//标准标识符
//	TxMessage.ExtId=data[1]; //扩展标识符
//  TxMessage.IDE=CAN_Id_Standard;//使用标准标识符
//  TxMessage.RTR=CAN_RTR_DATA;//为数据帧
//  TxMessage.DLC=data[2];	//	消息的数据长度
//	
//		for(i=0;i<data[2];i++)//取掉帧头 帧尾
//	{
//		TxMessage.Data[i]=data[i+3]; //第一个字节数据 
//		printf("发生数据：%X",TxMessage.Data[i]);
//	}
//	
//	CAN_Transmit(CAN1,&TxMessage); //发送数据
//}

void send_cmd(int* data)//将串口数据转发个CAN
{
	u8 i=0;
	CanTxMsg TxMessage;  
  TxMessage.StdId=data[1];	//标准标识符为0x00
  TxMessage.ExtId=data[1]; //扩展标识符0x0000
  TxMessage.IDE=CAN_Id_Standard;//使用标准标识符
  TxMessage.RTR=CAN_RTR_DATA;//为数据帧
  TxMessage.DLC=data[2];	//	消息的数据长度 2
	for(i=0;i<5;i++)//取掉帧头 帧尾
	{
		TxMessage.Data[i]=data[i+3]; //第一个字节数据 
		if(i>=data[2])
		{
			break;//结束发送
		}
	}
	CAN_Transmit(CAN1,&TxMessage); //发送数据
}

//通过串口外发CAN收到的信息
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

//发送两个字节数据函数
void Usart_SendHalfWord(USART_TypeDef* pUSARTx,uint16_t data)
{
	//发送十六位数据要分为两次来发送，先定义两个变量
	uint8_t temp_h,temp_l;//定义8位的变量（分别存储高8位和低8位）

	//首先取出高8位
	temp_h=(data&0xff00)>>8;//低八位先与0相&，低8位变为0再右移8位（0xff00共16位二进制）
	//再取出低8位
	temp_l=data&0xff;//取出低8位数据
	//16位的数据这样子就放到了两个变量里面（共16位）
	
	//调用固件库函数
	USART_SendData(pUSARTx,temp_h);//先发送高8位
	while(USART_GetFlagStatus(pUSARTx,USART_FLAG_TXE)==RESET);//等待数据发送完毕

	USART_SendData(pUSARTx,temp_l);//再发送低8位
	while(USART_GetFlagStatus(pUSARTx,USART_FLAG_TXE)==RESET);//等待数据发送完毕

}

/**************************************************************************
函数功能：发送一个字节
入口参数：发送的字节
返回  值：无
**************************************************************************/
void USART_SendByte(uint8_t Byte)
{
	USART_SendData(USART1,Byte);
	// 等待写入完成，写入完成之后会将标志位自动清0
	while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET);
}

/**************************************************************************
函数功能：发送指定大小的数组
入口参数：数组地址、数组大小
返回  值：无
**************************************************************************/
void USART_SendArray(uint8_t *Array,uint16_t Length)
	{
	uint8_t i = 0;
	for(i=0;i<Length;i++)
		{
		USART_SendData(USART1,Array[i]);
		// 等待写入完成，写入完成之后会将标志位自动清0
		while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET);
   	}
  }

/**************************************************************************
函数功能：发送指定大小的字符数组，被usartSendData函数调用
入口参数：数组地址、数组大小
返回  值：无
**************************************************************************/
void USART_Send_String(u8 *p,u16 sendSize)
{ 
	static int length =0;
	while(length<sendSize)
	{   
		//如果你使用不是USART1更改成相应的，比如USART3，这里有两处修改
		while( !(USART1->SR&(0x01<<7)) );//发送缓冲区为空
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
函数功能：电机的转速等信息进行打包，通过串口发送给Linux
入口参数：CAN接收数组
返回  值：无
**************************************************************************/
//void usartSendData(char *Can_RX_Buff)
//{
//	//协议数据缓存数组
//  char buf[20]={0};
//	int i;
//	//帧头
//	buf[0]=0x55;
//	buf[1]=0xaa;
//	//设置发送数据

//	for(i=2;i<20;i++)
//	{
//		buf[i]=Can_RX_Buff[i];                   // buf[2]data
//	}
//	//发送字符串数据
////	USART_Send_String(buf,sizeof(buf));
//   
//	for(i=0;i<20;i++)
//	{
//		USART_SendData(USART1,buf[i]);
//		printf("发送给CAN的数据%c ",buf[i]);
//	}
//}

/**********************************END***************************************/
