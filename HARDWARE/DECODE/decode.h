#ifndef __Decode_H
#define __Decode_H
#include "sys.h"

typedef union{
  u8 Bytes;
  struct {
    u8  _GearReq                      :2;
    u8  _HandBrakeReq                 :1;  
    u8  _TrumpetCtrl                  :1;                                       
    u8  _LightCtrl                    :2;                                       
    u8  _IgnitCtrl                    :1;                                       
    u8  _FlameoutCtrl                 :1; 
   // u8                              :1;                                       
   // u8                              :1;                                       
  } Bits;
}MFCCtrlByte0Def;


extern volatile MFCCtrlByte0Def  _MFCCtrlByte0;
#define MFCCtrlByte0            _MFCCtrlByte0.Bytes
#define GearReq                 _MFCCtrlByte0.Bits._GearReq 
#define HandBrakeCtrl           _MFCCtrlByte0.Bits._HandBrakeReq
#define TrumpetCtrl           _MFCCtrlByte0.Bits._TrumpetCtrl
#define LightCtrl               _MFCCtrlByte0.Bits._LightCtrl
#define IgnitCtrl               _MFCCtrlByte0.Bits._IgnitCtrl
#define FlameoutCtrl               _MFCCtrlByte0.Bits._FlameoutCtrl

/////////////////////////////////////////////////////////
#define GearReqChannel              5
#define HandBrakeCtrlChannel        8
#define LightCtrlChannel            7
#define Ignit_FlameoutCtrlChannel   3
#define Throttle_BrakeReqChannel    2
#define SteerReqChannel             1
#define TrumpetCtrlChannel          4

typedef union{
  u8 Bytes;
  struct {
    u8  _BCUStu                      :1;
    u8  _EngStu                      :1;  
    u8                               :1;                                       
    u8                               :1;                                       
    u8                              :1;                                       
    u8                              :1; 
    u8                              :1;                                       
    u8                              :1;                                       
  } Bits;
}MFCMonitorStusDef;
 extern volatile MFCMonitorStusDef  _MFCMonitorStus;
#define MFCMonitorStusByte            _MFCMonitorStus.Bytes
#define BCUStu                 _MFCMonitorStus.Bits._BCUStu
#define EngStu                 _MFCMonitorStus.Bits._EngStu 



#define SpeedSensorTeeth  52      //测速齿齿数为52
#define EngSpeedSensorTeeth  23      //测速齿齿数为23

extern volatile u8  ThrottleReq;//精度0.4%
extern volatile u8  BrakeReq;//精度0.4%
extern volatile int8_t  SteerReq;//分辨率1，偏移量-100
extern volatile int8_t  ManualSteerReq;//分辨率1，偏移量-100
extern volatile u16  LeftSpeed;//unit:rpm
extern volatile u16  RightSpeed;//unit:rpm
extern volatile u16  EngSpeed;//unit:rpm
extern u8 MFC2BCUbuf[8];
extern u8 MFC2BCU2buf[8];
extern volatile u8  MFC_ErrorCode;




u16 wheelSpdCal(u16 Timer_Capture_VAL,u16 TimerOVFcnt);
u16 EngSpdCal(u16 Timer_Capture_VAL,u16 TimerOVFcnt);
void SBUS2MFCdecode(void);
void ManualSteerReqScan(void);
void MFC_FeedBack(void);
void MFC2BCU2_(void);
void MFCReboot(void);
void MFCTest(void);

#endif


