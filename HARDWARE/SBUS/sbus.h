#ifndef _SBUS_H
#define _SBUS_H
#include "sys.h"
//extern u8 RC_LEN;//接收数组字长
//extern u16 sBUF[25];
//extern u16 Data[25];
////unsigned int ch[16];
//extern int data_ch[16];	

extern u8 RC_LEN;//接收数组字长
extern u16 sBUF[25];//缓冲buf
extern u16 Data[25];//存储BUF
extern u16 data_22[22];//计算转换过度
extern unsigned char	data_b[176];//二进制存储
extern int data_ch[16];//最终存储
extern int rt[16];
extern u8 uart2RxFlag;
extern u8 ChannelDataChangeFlg;
extern int lastdata_ch[16];//最终存储
//int fputc_1(int ch,FILE *f);

void sbus_init(u32 bound);
void Cal_RcData(void);
void pre_decode(void);


#endif
