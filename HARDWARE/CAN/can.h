#ifndef __CAN_H
#define __CAN_H	 
#include "sys.h"	    


//CAN接收RX0中断使能
#define CAN_RX0_INT_ENABLE	1		//0,不使能;1,使能.								    
										 							 				    
void CAN_Mode_Init(void);//CAN初始化
void can_heartbeat_tx(u8 can_id);//心跳帧
void can_tx_speed(u8 can_id,char Data1,char Data2,char Data3,char Data4);
void can_tx(u8 Data1,u8 Data2); 
//void Can_Send_Msg(u8* msg,u8 len,uint32_t CAN_ID);
void Can_Send_Msg(char* msg,u8 len,uint32_t CAN_ID);
void USB_LP_CAN1_RX0_IRQHandler(void);
int hexcharToInt(char c);
u8 Can_Receive_Msg(u8 *buf);							//接收数据
void can_tx_duty_usart(char can_id,char Data1,char Data2);
void can_tx_inf_usart(u8 can_id,char* Data);
double trapezoidal_speed(double target_speed, double current_speed);



#endif

















