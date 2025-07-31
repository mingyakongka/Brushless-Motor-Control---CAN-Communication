#include "stm32f10x.h"
#include "key.h"
#include "sys.h" 
#include "delay.h"
					    
//按键初始化函数  PB12 PB13 默认高电平 按下低电平
void KEY_Init(void) //IO初始化
{ 
 	GPIO_InitTypeDef GPIO_InitStructure;
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);//使能PORTB时钟
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_13;//KEY0-KEY1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
 	GPIO_Init(GPIOB,&GPIO_InitStructure);//初始化GPIOB12,13
}

u8 KEY_Scan(void)
{	 
	if((KEY0==0||KEY1==0))//按键按下
	{
		delay_ms(20);//去抖动 
		if(KEY0==0)
			return KEY0_PRES;
		else if(KEY1==0)
			return KEY1_PRES;
	}
	else if(KEY0==1&&KEY1==1)//没有按下    
 	  return 0;// 无按键按下
}
