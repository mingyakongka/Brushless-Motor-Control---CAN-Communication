#ifndef __KEY_H
#define __KEY_H	 
#include "sys.h"
 	 
#define KEY0  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12)//读取按键0
#define KEY1  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_13)//读取按键1

#define KEY0_PRES 	1	//KEY0按下
#define KEY1_PRES	  2	//KEY1按下

void KEY_Init(void);//IO初始化
//u8 KEY_Scan(u8);  	//按键扫描函数
u8 KEY_Scan(void);
#endif
