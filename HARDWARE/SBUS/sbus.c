#include "sys.h"
#include "sbus.h"
#include "usart.h"
#include "string.h"
#include "led.h"
/*
数据格式
[startbyte]	[data1]	[data2]	…	[data22]	[flags]	[endbyte]
0x0f						                                     0x00
100k波特率，8位数据位（在stm32中要选择9位），偶校验（EVEN)，2位停止位，无控流，25个字节。

contrl_flag标志位是用来检测控制器与px4是否断开的标志位：
contrl_flag=1：控制器与接收器保持连接；
contrl_flag=0：控制器与接收器断开（失控），px4会控制电机停转。
*/

//串口2 与接收机通讯
//硬件接口 TX:PA2  RX:PA3
u8 contrl_flag=1;//判断遥控器是否连接
u8 RC_LEN=0;//接收数组字长
u16 sBUF[25];//缓冲buf
u16 Data[25];//存储BUF

u16 data_22[22];//计算转换过度 保留控制位
unsigned char	data_b[176];//二进制存储

//实际只有六个通道
int data_ch[16]={0};//最终存储 有符号数
static uint16_t CH[6]={0};

int rt[16]={0};
uint16_t values[16]={0};

u8 uart2RxFlag;//串口2接收状态标志位
u8 ChannelDataChangeFlg=1;//通道1的数据标志位

int lastdata_ch[16]={0};//最终存储


//本质就是串口程序  8个数据位 偶校验 2个停止位 100K
void sbus_init(u32 bound)//SBUS波特率要求100K 
{
	//结构体命名
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	//串口2
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	//USART1_TX   PA.2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA.2
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	//USART1_RX	  PA.3
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;//PA.3
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
  GPIO_Init(GPIOA, &GPIO_InitStructure);  
	//NVIC配置 2 2
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2 ;//抢占优先级2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =2;		//子优先级2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);

  //USART 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//设置波特率; 100K
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_2;//2个停止位
	USART_InitStructure.USART_Parity = USART_Parity_Even;//偶校验
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
 
  USART_Init(USART2, &USART_InitStructure); //初始化串口
	//Receive Data register not empty interrupt
  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//Enables  the specified USART interrupts.
  USART_Cmd(USART2, ENABLE);                    //使能串口 
}


//串口调试助手不管接收到的是什么数据，都会转化为字符
//只有发送十六进制数据，串口助手使用十六进制形式接收数据时才不是字符
//串口2中断服务程序 接收SBUS数组
//感觉一次是接受一帧
void USART2_IRQHandler(void) 
{
  u8 i;
	if(USART_GetITStatus(USART2,USART_IT_RXNE)!= RESET)//串口接收到数据
	{
		USART_ClearITPendingBit(USART2,USART_IT_RXNE);//首先清除中断标志位
		//返回最新收到的数据
	  sBUF[RC_LEN++]=USART_ReceiveData(USART2);//把串口2的数据给到SBUS
		
		if(sBUF[0]==0x0f&&RC_LEN==24&&sBUF[24]==0x00)//如果帧头帧尾和长度满足 传输
		{
		  RC_LEN=0;//接收长度置零 等待下次赋值
			//清零数组 可以再简化
			memset(Data,0,sizeof(Data)); //数组清零
				if(i==0) 
				{
					uart2RxFlag=0;//标志位清零
				}
			//把暂存数组的值转移到数据数组里
			for(i=0;i<25;i++)
			{
			  Data[i]=sBUF[i];
			}
			uart2RxFlag=1;//接收完正确的数组
			contrl_flag=Data[23];//标志位
		}
	}
}

////把接收的数据放到Data[i]数组里
//void pre_decode(void)
//{
//	  u8 i;
//		if(sBUF[0]==0x0f&&RC_LEN==24&&sBUF[24]==0x00)//如果帧头帧尾和长度满足 传输
//		{
//		  RC_LEN=0;//接收长度置零 等待下次赋值
//			//清零数组 可以再简化
//		  for(i=0;i<25;i++)
//			{
//				if(i==0) 
//				{
//					uart2RxFlag=0;//标志位清零
//				}
//			  Data[i]=0;//先给数据数组清零了 才方便后面重新赋值
//			}
//			//把暂存数组的值转移到数据数组里
//			for(i=0;i<25;i++)
//			{
//			  Data[i]=sBUF[i];
//			}
//			uart2RxFlag=1;//接收完正确的数组
//			contrl_flag=Data[23];//标志位
//}
//}

//处理好后最终的数据为data_ch[j]
//解析收到的通道数据
//循环用多了导致处理很慢
void Cal_RcData(void)
{
	u8 i,j=0;
	//摘取22位通道数据 
	for(i=1;i<23;i++)//去掉帧头 帧尾 
	{
		data_22[j++]=Data[i];
	}	
	//转换位2进制存储
	for(i=0;i<22;i++)
	{
		int k=1;
		for(j=0;j<8;j++)
		{	 
			data_b[j+8*i]=(data_22[i]&k)>>j;
			k<<=1;	
		} 	
	}
	  memset(data_ch,0, sizeof data_ch); //数组清零
	//转换为SBUS要求的格式 6个通道 16
	for(j=0;j<6;j++)
	{
		for(i=10;i>0;i--)
		{
			data_ch[j]=data_ch[j]|(data_b[i+j*11]<<i);
		}     
	}	
}





