#ifndef __MBOTLINUXUSART__
#define __MBOTLINUXUSART__
#include <sys.h>	

void usartSendData(char *Can_RX_Buff);
void usartReceiveData(void);
void USART_Send_String(u8 *p,u16 sendSize);

//�����λѭ������У�飬�õ�У��ֵ��һ���̶�����֤���ݵ���ȷ��
unsigned char getCrc8(unsigned char *ptr, unsigned short len); 

#endif
