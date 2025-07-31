#include "led.h"
    
//LED IO初始化 PB14 PB15共阳极
void LED_Init(void)
{
 GPIO_InitTypeDef  GPIO_InitStructure;
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);	 //使能PB端口时钟
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15|GPIO_Pin_14;	//LED0-->PB14  LED1-->PB15端口配置
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
 GPIO_Init(GPIOB,&GPIO_InitStructure);					       //根据设定参数初始化GPIOB
 GPIO_SetBits(GPIOB,GPIO_Pin_15|GPIO_Pin_14);					//输出高 LED不亮
}
 
