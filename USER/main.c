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

//״̬��־λ
extern int CAN_RX_STA;//CAN���ܱ�־λ
extern u8 RxFlag;//����1����״̬��־λ
extern u8 contrl_flag;//ң��������״̬��־λ
extern int  Can_RX_Buff[16];//���ջ��� 

//ͨ�ſ�����ز���
int real_data[6];
char buffer [33]; //���ڴ��ת���õ�ʮ�������ַ������ɸ�����Ҫ���峤��
int i=0,k=0;
extern int data[16];
u8 flag=0;

//������ز���
int speed_rx=0;
int vol_rx=0;
int duty_rx=0;
int cur_tx=0;
int speed_tx=0;
int duty_tx=0;	
int speed_h=0;
int speed_l=0;	

//ң���������ֲ���
int R_speed=0;
int L_speed=0;
int R_speed_hex=0;
int L_speed_hex=0;

char Hex[10]="0";//���ڱ���sprintfת������ ��ֹ��� char �˸��ֽ� ����
char L_Hex[10];//���ڱ���sprintfת������ ��ֹ��� char �˸��ֽ� ����
char R_Hex[10];//���ڱ���sprintfת������ ��ֹ��� char �˸��ֽ� ����
char L1='0';//���ڱ���ƴ�ӵ�ǰ����16��������
char L2='0';//���ڱ���ƴ�ӵĺ�����16��������
char L3='0';//���ڱ���ƴ�ӵ�ǰ����16��������
char L4='0';//���ڱ���ƴ�ӵĺ�����16��������

char R1='0';//���ڱ���ƴ�ӵ�ǰ����16��������
char R2='0';//���ڱ���ƴ�ӵĺ�����16��������
char R3='0';//���ڱ���ƴ�ӵ�ǰ����16��������
char R4='0';//���ڱ���ƴ�ӵĺ�����16��������


 int main(void)
 {
	 
	delay_init();	//��ʱ������ʼ��	 
  LED_Init();//LED ��ʼ��	 
	uart_init(115200);//���ڳ�ʼ��Ϊ115200 �Խ���λ��
	TIM3_Int_Init(4999,7199);//10Khz�ļ���Ƶ�ʣ�������5000Ϊ500ms   ��������֡ LED1
	sbus_init(100000); //SBUS�Ĳ�����Ϊ100K �Խ�ң����
	CAN_Mode_Init();//CAN��ʼ������ģʽ���Խ�����������250Kbps 
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�

	 while(1)
	{
		if(CAN_RX_STA==1)//ת��CAN���յ���Ϣ
		{
		/*---���ռ�ձ� ��ѹ ת����Ϣ���ϱ�---*/
		 CAN_RX_STA=0;//��־λ����
	   send_report(Can_RX_Buff);//ת��CAN ��������
			
		/*---�Խ��ܵ���Ϣ���н��� ����Ϊ10����---*/
			
	/*
			5A 01 04 0F 01 00 00 00 5B//���ز�ѯת����Ϣ
			5A 01 04 /0F 02/ 00 00 00 5B//���ز�ѯռ�ձ���Ϣ
			5A 01 04 0F 04 00 00 00 5B //���ز�ѯ��ѹ��Ϣ
	*/
			
	if(Can_RX_Buff[0]==0x01)//��ߵ��
	 {
		switch (Can_RX_Buff[3]) 
			{
        case 0x01://ת����Ϣ
						speed_rx=Can_RX_Buff[4];//���λ
						speed_rx=((speed_rx<<8)+Can_RX_Buff[5]);
				    speed_rx=((speed_rx<<8)+Can_RX_Buff[6]);
				    speed_rx=((speed_rx<<8)+Can_RX_Buff[7]);
//            printf("��ת����Ϣ��%X\r\n",speed_rx);
            break;
        case 0x02://ռ�ձ���Ϣ				
						duty_rx=Can_RX_Buff[4];//���λ
						duty_rx=((duty_rx<<8)+Can_RX_Buff[5]);
//            printf("��ռ�ձ���Ϣ��%X\r\n",duty_rx);
            break;
        case 0x04://��ѹ��Ϣ
						vol_rx=Can_RX_Buff[4];//���λ
						vol_rx=((vol_rx<<8)+Can_RX_Buff[5]);
//            printf("���ѹ��Ϣ��%X\r\n",vol_rx);
            break;
			}
		}	
		if(Can_RX_Buff[0]==0x02)//�ұߵ��
	{	
		switch (Can_RX_Buff[3]) 
			{
        case 0x01://ת����Ϣ
						speed_rx=Can_RX_Buff[4];//���λ
						speed_rx=((speed_rx<<8)+Can_RX_Buff[5]);
				    speed_rx=((speed_rx<<8)+Can_RX_Buff[6]);
				    speed_rx=((speed_rx<<8)+Can_RX_Buff[7]);
//            printf("��ת����Ϣ��%X\r\n",speed_rx);
            break;
        case 0x02://ռ�ձ���Ϣ				
						duty_rx=Can_RX_Buff[4];//���λ
						duty_rx=((duty_rx<<8)+Can_RX_Buff[5]);
//            printf("��ռ�ձ���Ϣ��%X\r\n",duty_rx);
            break;
        case 0x04://��ѹ��Ϣ
						vol_rx=Can_RX_Buff[4];//���λ
						vol_rx=((vol_rx<<8)+Can_RX_Buff[5]);
//            printf("�ҵ�ѹ��Ϣ��%X\r\n",vol_rx);
            break;
			}
		}	
	}	
		
/*
DATA[0]��0x02 ����Ϊ�ٶȿ��ƣ�0x0A ����Ϊ���ü��ٶȣ�0x10 ����Ϊ���ü��ٶȡ�
DATA[1]�������ٶ�Ϊ int �ͣ�4 ���ֽڡ��ٶ�ֵ�� 24 λ��
DATA[2]�������ٶ�Ϊ int �ͣ�4 ���ֽڡ��ٶ�ֵ�� 16 λ��
DATA[3]�������ٶ�Ϊ int �ͣ�4 ���ֽڡ��ٶ�ֵ�� 8 λ��
DATA[4]�������ٶ�Ϊ int �ͣ�4 ���ֽڡ��ٶ�ֵ�� 8 λ
*/
	
/*
�����ռ�ձ�95%-ת��43300erpm - ת�ٷ�Χ��-43300---+43300
      ��Ӧ��ң���������ϣ�800 ���£�800 ת������ϵ��54
	
			��ʼֵ            986: 1048: 1048: 1000: 1000: 1000:
			�����������ϴ�����986: 1048: 1846: 1042: 1000: 1000:CHANL3
			�����������´�����986: 1048: 248: 992: 1000: 1000:CHANL3
			��ʼֵ            986: 1048: 1048: 1000: 1000: 1000:
			�����������ϴ�����980: 1846: 1048: 1002: 1000: 1000:CHANL2
			�����������´�����1088: 248: 1048: 1002: 1000: 1000:CHANL2 
			contrl_flag δ���ӣ�16   ���ӣ�0
*/
	
////ң�����ӹ� uart2RxFlag�������  contrl_flagң��������
if(uart2RxFlag==1&&contrl_flag==0)//��ɽ���
	{
		uart2RxFlag=0;//���ձ�־λ����		
		flag=1;//ң������־λ���� ���δ��ڿ���
		Cal_RcData();//ת��ΪSBUS��ʽ
/*
INT 4�ֽ�		��Ҫ����� data_ch[k]-1000�������ֵ�� �з�����	
ռ�ձ���������ת�������ͷ�ת��������-1000~1000����Ӧ�������ת�ٺ��������ת�١�			
*/	
		//�������� chanl 2
		real_data[0]=(data_ch[2]-1000);//��ȥ����ʱ���ֵ
		
		//��������Ư������
		if(real_data[0]>(-10)&&real_data[0]<10)
		{
			real_data[0]=0;
		}
		
		L_speed=real_data[0]*108;//ת��Ϊ��Ӧ��ת��erpm
		

			if(L_speed>43300||L_speed<-43300)//����������
		{
			L_speed=0;
		}
			sprintf(L_Hex,"%08X",L_speed);//ת��Ϊʮ������ ֻת��Ϊ4���ֽ� ��߸�λ����
			L1=(char)((hexcharToInt(L_Hex[0])<<4)|hexcharToInt(L_Hex[1]));
			//��sprintfת����ĵ�һ���͵ڶ���16������ƴ�ӳ�һ��16������
			L2=(char)((hexcharToInt(L_Hex[2])<<4)|hexcharToInt(L_Hex[3]));
			//��sprintfת����ĵ�һ���͵ڶ���16������ƴ�ӳ�һ��16������
			L3=(char)((hexcharToInt(L_Hex[4])<<4)|hexcharToInt(L_Hex[5]));
			//��sprintfת����ĵ�һ���͵ڶ���16������ƴ�ӳ�һ��16������
			L4=(char)((hexcharToInt(L_Hex[6])<<4)|hexcharToInt(L_Hex[7]));
			//��sprintfת����ĵ�һ���͵ڶ���16������ƴ�ӳ�һ��16������
			can_tx_speed(01,L1,L2,L3,L4);
		
		
		//�������� chanl 1
		real_data[1]=(data_ch[1]-1000);//��ȥ����ʱ���ֵ
		
		if(real_data[1]>(-10)&&real_data[1]<10)
		{
			real_data[1]=0;
		}
		R_speed=real_data[1]*108;//ת��Ϊ��Ӧ��ת��erpm
		
			if(R_speed>43300||R_speed<-43300)//����������
		{
			R_speed=0;
		}
			sprintf(R_Hex,"%08X",R_speed);//ת��Ϊʮ������ ֻת��Ϊ4���ֽ� ��߸�λ����
			R1=(char)((hexcharToInt(R_Hex[0])<<4)|hexcharToInt(R_Hex[1]));
			//��sprintfת����ĵ�һ���͵ڶ���16������ƴ�ӳ�һ��16������
			R2=(char)((hexcharToInt(R_Hex[2])<<4)|hexcharToInt(R_Hex[3]));
			//��sprintfת����ĵ�һ���͵ڶ���16������ƴ�ӳ�һ��16������
			R3=(char)((hexcharToInt(R_Hex[4])<<4)|hexcharToInt(R_Hex[5]));
			//��sprintfת����ĵ�һ���͵ڶ���16������ƴ�ӳ�һ��16������
			R4=(char)((hexcharToInt(R_Hex[6])<<4)|hexcharToInt(R_Hex[7]));
			//��sprintfת����ĵ�һ���͵ڶ���16������ƴ�ӳ�һ��16������
			can_tx_speed(02,R1,R2,R3,R4);
	}
	
/////��λ��ͨѶ����
/*--------------------------------����Э��-----------------------------------
//---------------- 5a CAN_ID DATA_len 0X 00 00 00 00 5b ----------------------
	                  0     1      2     3  4  5  6  7  8
	ռ�ձȿ���  5a 01 3 03 00 64 5b ����Ŀ��ռ�ձ�100��
//����ͷ0x5a + CAN_ID + ���ݳ���+ 0X01ռ�ձȿ��� + ����β0d5b ���� �Ÿ��ֽ�
--------------------------------------------------------------------------*/
	if(flag==0)//����λ
	{
		if(USART_RX_STA==1)//���ڽ��յ�������ң����δ����
		{
      send_cmd(data);//���ݳ������Ϊ5
			USART_RX_STA=0;//��־λ��λ
		}
	}
	if(contrl_flag!=0)//ң�����Ͽ� �ָ�ͨѶ����
	{
		flag=0;
	}
	
	  }
     }
	

	
	

