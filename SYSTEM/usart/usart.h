#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "sys.h" 

#define USART_REC_LEN  			200  	//�����������ֽ��� 200
#define EN_USART1_RX 			1		//ʹ�ܣ�1��/��ֹ��0������1����
	  	

extern u8 USART_RX_STA;         		//����״̬���

//����봮���жϽ��գ��벻Ҫע�����º궨��
void uart_init(u32 bound);
void USART_SendByte(uint8_t Byte);
void USART_SendArray(uint8_t *Array,uint16_t Length);
void USART_SendString(uint8_t *String);
void USART_SendNum(uint32_t Num,uint16_t Length);
void send_cmd(int* data);
void send_report(int* data);
#endif


