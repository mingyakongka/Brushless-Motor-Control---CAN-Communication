#ifndef _SBUS_H
#define _SBUS_H
#include "sys.h"
//extern u8 RC_LEN;//���������ֳ�
//extern u16 sBUF[25];
//extern u16 Data[25];
////unsigned int ch[16];
//extern int data_ch[16];	

extern u8 RC_LEN;//���������ֳ�
extern u16 sBUF[25];//����buf
extern u16 Data[25];//�洢BUF
extern u16 data_22[22];//����ת������
extern unsigned char	data_b[176];//�����ƴ洢
extern int data_ch[16];//���մ洢
extern int rt[16];
extern u8 uart2RxFlag;
extern u8 ChannelDataChangeFlg;
extern int lastdata_ch[16];//���մ洢
//int fputc_1(int ch,FILE *f);

void sbus_init(u32 bound);
void Cal_RcData(void);
void pre_decode(void);


#endif
