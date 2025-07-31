#include "led.h"
    
//LED IO��ʼ�� PB14 PB15������
void LED_Init(void)
{
 GPIO_InitTypeDef  GPIO_InitStructure;
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);	 //ʹ��PB�˿�ʱ��
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15|GPIO_Pin_14;	//LED0-->PB14  LED1-->PB15�˿�����
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
 GPIO_Init(GPIOB,&GPIO_InitStructure);					       //�����趨������ʼ��GPIOB
 GPIO_SetBits(GPIOB,GPIO_Pin_15|GPIO_Pin_14);					//����� LED����
}
 
