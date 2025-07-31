#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "sys.h" 

#define USART_REC_LEN  			200  	//定义最大接收字节数 200
#define EN_USART1_RX 			1		//使能（1）/禁止（0）串口1接收
	  	

extern u8 USART_RX_STA;         		//接收状态标记

//如果想串口中断接收，请不要注释以下宏定义
void uart_init(u32 bound);
void USART_SendByte(uint8_t Byte);
void USART_SendArray(uint8_t *Array,uint16_t Length);
void USART_SendString(uint8_t *String);
void USART_SendNum(uint32_t Num,uint16_t Length);
void send_cmd(int* data);
void send_report(int* data);
#endif


