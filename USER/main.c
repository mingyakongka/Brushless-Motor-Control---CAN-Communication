#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "usart.h"
#include "sbus.h"
#include "can.h"
#include "timer.h"
#include <stdlib.h>
#include "stdio.h"
#include <stdint.h>
#include <string.h>

//状态标志位
extern int CAN_RX_STA;//CAN接受标志位
extern u8 RxFlag;//串口1接收状态标志位
extern u8 contrl_flag;//遥控器连接状态标志位
extern int  Can_RX_Buff[16];//接收缓存 

//通信控制相关参数
int real_data[6];
char buffer [33]; //用于存放转换好的十六进制字符串，可根据需要定义长度
int i=0,k=0;
extern int data[16];
u8 flag=0;

//控制相关参数
int speed_rx=0;
int vol_rx=0;
int duty_rx=0;
int cur_tx=0;
int speed_tx=0;
int duty_tx=0;	
int speed_h=0;
int speed_l=0;	

//遥控器左右手参数
int R_speed=0;
int L_speed=0;
int R_speed_hex=0;
int L_speed_hex=0;

char Hex[10]="0";//用于保存sprintf转换后结果 防止溢出 char 八个字节 左手
char L_Hex[10];//用于保存sprintf转换后结果 防止溢出 char 八个字节 左手
char R_Hex[10];//用于保存sprintf转换后结果 防止溢出 char 八个字节 右手
char L1='0';//用于保存拼接的前两个16进制数据
char L2='0';//用于保存拼接的后两个16进制数据
char L3='0';//用于保存拼接的前两个16进制数据
char L4='0';//用于保存拼接的后两个16进制数据

char R1='0';//用于保存拼接的前两个16进制数据
char R2='0';//用于保存拼接的后两个16进制数据
char R3='0';//用于保存拼接的前两个16进制数据
char R4='0';//用于保存拼接的后两个16进制数据


 int main(void)
 {
	 
	delay_init();	//延时函数初始化	 
  LED_Init();//LED 初始化	 
	uart_init(115200);//串口初始化为115200 对接上位机
	TIM3_Int_Init(4999,7199);//10Khz的计数频率，计数到5000为500ms   发送心跳帧 LED1
	sbus_init(100000); //SBUS的波特率为100K 对接遥控器
	CAN_Mode_Init();//CAN初始化正常模式，对接驱动波特率250Kbps 
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置NVIC中断分组2:2位抢占优先级，2位响应优先级

	 while(1)
	{
		if(CAN_RX_STA==1)//转发CAN接收到信息
		{
		/*---完成占空比 电压 转速信息的上报---*/
		 CAN_RX_STA=0;//标志位清零
	   send_report(Can_RX_Buff);//转发CAN 接收数据
			
		/*---对接受的信息进行解析 并换为10进制---*/
			
	/*
			5A 01 04 0F 01 00 00 00 5B//返回查询转速信息
			5A 01 04 /0F 02/ 00 00 00 5B//返回查询占空比信息
			5A 01 04 0F 04 00 00 00 5B //返回查询电压信息
	*/
			
	if(Can_RX_Buff[0]==0x01)//左边电机
	 {
		switch (Can_RX_Buff[3]) 
			{
        case 0x01://转速信息
						speed_rx=Can_RX_Buff[4];//最高位
						speed_rx=((speed_rx<<8)+Can_RX_Buff[5]);
				    speed_rx=((speed_rx<<8)+Can_RX_Buff[6]);
				    speed_rx=((speed_rx<<8)+Can_RX_Buff[7]);
//            printf("左转速信息是%X\r\n",speed_rx);
            break;
        case 0x02://占空比信息				
						duty_rx=Can_RX_Buff[4];//最高位
						duty_rx=((duty_rx<<8)+Can_RX_Buff[5]);
//            printf("左占空比信息是%X\r\n",duty_rx);
            break;
        case 0x04://电压信息
						vol_rx=Can_RX_Buff[4];//最高位
						vol_rx=((vol_rx<<8)+Can_RX_Buff[5]);
//            printf("左电压信息是%X\r\n",vol_rx);
            break;
			}
		}	
		if(Can_RX_Buff[0]==0x02)//右边电机
	{	
		switch (Can_RX_Buff[3]) 
			{
        case 0x01://转速信息
						speed_rx=Can_RX_Buff[4];//最高位
						speed_rx=((speed_rx<<8)+Can_RX_Buff[5]);
				    speed_rx=((speed_rx<<8)+Can_RX_Buff[6]);
				    speed_rx=((speed_rx<<8)+Can_RX_Buff[7]);
//            printf("右转速信息是%X\r\n",speed_rx);
            break;
        case 0x02://占空比信息				
						duty_rx=Can_RX_Buff[4];//最高位
						duty_rx=((duty_rx<<8)+Can_RX_Buff[5]);
//            printf("右占空比信息是%X\r\n",duty_rx);
            break;
        case 0x04://电压信息
						vol_rx=Can_RX_Buff[4];//最高位
						vol_rx=((vol_rx<<8)+Can_RX_Buff[5]);
//            printf("右电压信息是%X\r\n",vol_rx);
            break;
			}
		}	
	}	
		
/*
DATA[0]：0x02 代表为速度控制，0x0A 代表为设置加速度，0x10 代表为设置减速度。
DATA[1]：控制速度为 int 型，4 个字节。速度值高 24 位。
DATA[2]：控制速度为 int 型，4 个字节。速度值高 16 位。
DATA[3]：控制速度为 int 型，4 个字节。速度值高 8 位。
DATA[4]：控制速度为 int 型，4 个字节。速度值高 8 位
*/
	
/*
电机：占空比95%-转速43300erpm - 转速范围：-43300---+43300
      对应与遥控器：向上：800 向下：800 转换比例系数54
	
			初始值            986: 1048: 1048: 1000: 1000: 1000:
			左手油门向上打死：986: 1048: 1846: 1042: 1000: 1000:CHANL3
			左手油门向下打死：986: 1048: 248: 992: 1000: 1000:CHANL3
			初始值            986: 1048: 1048: 1000: 1000: 1000:
			右手油门向上打死：980: 1846: 1048: 1002: 1000: 1000:CHANL2
			右手油门向下打死：1088: 248: 1048: 1002: 1000: 1000:CHANL2 
			contrl_flag 未连接：16   连接：0
*/
	
////遥控器接管 uart2RxFlag接收完成  contrl_flag遥控器连接
if(uart2RxFlag==1&&contrl_flag==0)//完成接收
	{
		uart2RxFlag=0;//接收标志位清零		
		flag=1;//遥控器标志位拉高 屏蔽串口控制
		Cal_RcData();//转换为SBUS格式
/*
INT 4字节		需要输出的 data_ch[k]-1000（归零的值） 有符号数	
占空比正数就正转，负数就反转。量程是-1000~1000，对应反向最大转速和正向最大转速。			
*/	
		//左手油门 chanl 2
		real_data[0]=(data_ch[2]-1000);//减去归零时候的值
		
		//消除归零漂移问题
		if(real_data[0]>(-10)&&real_data[0]<10)
		{
			real_data[0]=0;
		}
		
		L_speed=real_data[0]*108;//转换为对应的转速erpm
		

			if(L_speed>43300||L_speed<-43300)//超过限制了
		{
			L_speed=0;
		}
			sprintf(L_Hex,"%08X",L_speed);//转换为十六进制 只转换为4个字节 左边高位补零
			L1=(char)((hexcharToInt(L_Hex[0])<<4)|hexcharToInt(L_Hex[1]));
			//将sprintf转换后的第一个和第二个16进制数拼接成一个16进制数
			L2=(char)((hexcharToInt(L_Hex[2])<<4)|hexcharToInt(L_Hex[3]));
			//将sprintf转换后的第一个和第二个16进制数拼接成一个16进制数
			L3=(char)((hexcharToInt(L_Hex[4])<<4)|hexcharToInt(L_Hex[5]));
			//将sprintf转换后的第一个和第二个16进制数拼接成一个16进制数
			L4=(char)((hexcharToInt(L_Hex[6])<<4)|hexcharToInt(L_Hex[7]));
			//将sprintf转换后的第一个和第二个16进制数拼接成一个16进制数
			can_tx_speed(01,L1,L2,L3,L4);
		
		
		//右手油门 chanl 1
		real_data[1]=(data_ch[1]-1000);//减去归零时候的值
		
		if(real_data[1]>(-10)&&real_data[1]<10)
		{
			real_data[1]=0;
		}
		R_speed=real_data[1]*108;//转换为对应的转速erpm
		
			if(R_speed>43300||R_speed<-43300)//超过限制了
		{
			R_speed=0;
		}
			sprintf(R_Hex,"%08X",R_speed);//转换为十六进制 只转换为4个字节 左边高位补零
			R1=(char)((hexcharToInt(R_Hex[0])<<4)|hexcharToInt(R_Hex[1]));
			//将sprintf转换后的第一个和第二个16进制数拼接成一个16进制数
			R2=(char)((hexcharToInt(R_Hex[2])<<4)|hexcharToInt(R_Hex[3]));
			//将sprintf转换后的第一个和第二个16进制数拼接成一个16进制数
			R3=(char)((hexcharToInt(R_Hex[4])<<4)|hexcharToInt(R_Hex[5]));
			//将sprintf转换后的第一个和第二个16进制数拼接成一个16进制数
			R4=(char)((hexcharToInt(R_Hex[6])<<4)|hexcharToInt(R_Hex[7]));
			//将sprintf转换后的第一个和第二个16进制数拼接成一个16进制数
			can_tx_speed(02,R1,R2,R3,R4);
	}
	
/////上位机通讯控制
/*--------------------------------接收协议-----------------------------------
//---------------- 5a CAN_ID DATA_len 0X 00 00 00 00 5b ----------------------
	                  0     1      2     3  4  5  6  7  8
	占空比控制  5a 01 3 03 00 64 5b 设置目标占空比100。
//数据头0x5a + CAN_ID + 数据长度+ 0X01占空比控制 + 数据尾0d5b 定长 九个字节
--------------------------------------------------------------------------*/
	if(flag==0)//控制位
	{
		if(USART_RX_STA==1)//串口接收到数据且遥控器未连接
		{
      send_cmd(data);//数据长度最大为5
			USART_RX_STA=0;//标志位复位
		}
	}
	if(contrl_flag!=0)//遥控器断开 恢复通讯控制
	{
		flag=0;
	}
	
	  }
     }
	

	
	

