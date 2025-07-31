#ifndef __MBOTLINUXUSART__
#define __MBOTLINUXUSART__
#include <sys.h>	

void usartSendData(char *Can_RX_Buff);
void usartReceiveData(void);
void USART_Send_String(u8 *p,u16 sendSize);

//计算八位循环冗余校验，得到校验值，一定程度上验证数据的正确性
unsigned char getCrc8(unsigned char *ptr, unsigned short len); 

#endif
