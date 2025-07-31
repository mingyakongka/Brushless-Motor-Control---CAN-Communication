SBUS协议：使用的是串口的通讯，直接可以用串口发出，但有几点仍需要注意：

1.串口配置：100k波特率，8E2模式，无控流，一帧数据一共25个字节
8E2:
8:8个数据位
E:偶校验
2:两个停止位

2.协议格式：（8个数据位）
[startbyte] [data1][data2]…[data22][flags][endbyte]
startbyte=0x0f;
endbyte=0x00;

3.flags标志位

4.硬件电路连接
  SBUS采用负逻辑，所以无论接收还是发送都要进行硬件取反（注意，一定要硬件取反）

5.编码原理：
  一个信息是二进制的11bit，比如1111 1111 111就可以表示一个信息，一共16个这样的信息，按照顺序将这16个信息依次排成一串，得到一个176bit（11 *16）的数据，也就是22字节（176 / 8 = 22）的数据，再加上数据头数据尾校验位就组成了一个要通过串口传送的信息。每隔4或者14ms就传送一个这样的信息

   所以这16个信息每一个所能表示的最大值是2^11 = 2048，也就是他的精度。

306-1693


CAN通讯的速率与USB转串口的波特率也要匹配
CAN的通讯速率不匹配也无法正常进行通讯



///*发控制帧的数据-占空比*/
//void can_control_tx(u8 can_id,u8 data_1,u8 data_2)
//{ 
//  CanTxMsg TxMessage;  
//	//id
//  TxMessage.StdId=can_id;	//标准标识符为0x00
//  TxMessage.ExtId=can_id; //扩展标识符0x0000
//	//标准帧 数据帧
//  TxMessage.IDE=CAN_ID_EXT;//使用标准标识符
//  TxMessage.RTR=CAN_RTR_DATA;//为数据帧
//	//帧长度
//  TxMessage.DLC=3;	//	消息的数据长度为2个字节
//	//数据信息
//  TxMessage.Data[0]=3; //占空比控制
//	TxMessage.Data[1]=data_1; //占空比控制
//	TxMessage.Data[2]=data_2; //占空比控制
//  CAN_Transmit(CAN1,&TxMessage); //发送数据
//}

///*发控制帧的数据-刹车*/
//void can_stop_tx(u8 can_id,u8 data_1,u8 data_2)
//{ 
//  CanTxMsg TxMessage;  
//	//id
//  TxMessage.StdId=can_id;	//标准标识符为0x00
//  TxMessage.ExtId=can_id; //扩展标识符0x0000
//	//标准帧 数据帧
//  TxMessage.IDE=CAN_ID_EXT;//使用标准标识符
//  TxMessage.RTR=CAN_RTR_DATA;//为数据帧
//	//帧长度
//  TxMessage.DLC=3;	//	消息的数据长度为2个字节
//	//数据信息
//  TxMessage.Data[0]=8; //占空比控制
//	TxMessage.Data[1]=data_1; //刹车电流控制
//	TxMessage.Data[2]=data_2; //刹车电流控制
//  CAN_Transmit(CAN1,&TxMessage); //发送数据
//}
