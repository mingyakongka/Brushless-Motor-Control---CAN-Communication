#include "stm32f10x.h"
#include "key.h"
#include "sys.h" 
#include "delay.h"
					    
//������ʼ������  PB12 PB13 Ĭ�ϸߵ�ƽ ���µ͵�ƽ
void KEY_Init(void) //IO��ʼ��
{ 
 	GPIO_InitTypeDef GPIO_InitStructure;
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);//ʹ��PORTBʱ��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_13;//KEY0-KEY1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //���ó���������
 	GPIO_Init(GPIOB,&GPIO_InitStructure);//��ʼ��GPIOB12,13
}

u8 KEY_Scan(void)
{	 
	if((KEY0==0||KEY1==0))//��������
	{
		delay_ms(20);//ȥ���� 
		if(KEY0==0)
			return KEY0_PRES;
		else if(KEY1==0)
			return KEY1_PRES;
	}
	else if(KEY0==1&&KEY1==1)//û�а���    
 	  return 0;// �ް�������
}
