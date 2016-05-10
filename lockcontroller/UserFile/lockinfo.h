/**
  ******************************************************************************
  * @file    lockinfo.h
  * @author  CSIS Team
  * @version V1.0.3
  * @date    17-April-2016
  * @brief   Header file of LOCK INFORMATION module.
  ******************************************************************************
  **/
/* Define to prevent recursive inclusion -------------------------------------*/	
 #ifndef __LOCKINFO_H
 #define __LOCKINFO_H
 
 #ifdef __cplusplus
 extern "C" {
#endif 
/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx.h"
#include "Legacy/stm32_hal_legacy.h"
#include <stdio.h>
#include "user_usart.h"
#include "tim.h"
	 
#define DoorOpenState    1	 //设置门开状态为1
#define DoorClosedState  0	 //设置门关状态为0
#define MainlockOpenState    1	 //设置主锁舌开状态为1
#define MainlockClosedState  0	 //设置主锁舌关状态为0
#define AfflockOpenState    1	 //设置副锁舌开状态为1
#define AfflockClosedState  0	 //设置副锁舌关状态为0
	 
#define UserCodeByteSize    6	 //用户码个数
	 

 
//==================状态返回指令类型CmdStatusBackType================================//	
//============离站状态返回指令====================	 
#define LockClosedStatus_Success		  ((uint8_t)0x13)
#define LockClosedStatus_Error		    ((uint8_t)0x93)
#define LockClosedStatus_CmdError		  ((uint8_t)0xD3)
//============开锁状态返回指令====================	 
#define LockOpenStatus_Success        ((uint8_t)0x52)
#define LockOpenStatus_Error					((uint8_t)0x92)
#define LockOpenStatus_CmdError				((uint8_t)0xd2)
#define LOckOpenStatus_PlatError			((uint8_t)0xf2)
//============新建站状态返回指令==================
#define NewStationStatus_Success			((uint8_t)0x1E)
#define NewStationStatus_CmdSuccess		((uint8_t)0x5E)
#define NewStationStatus_Error				((uint8_t)0x9E)
#define NewStationStatus_CmdError			((uint8_t)0xDE)
//============门锁状态返回指令==================
#define DoorLockStatus_CmdError			  ((uint8_t)0xFF)

//===================门锁状态返回=====================================================//
#define  ExistLockingTab2							((uint8_t)0x80)
#define  ExistDoorMag2								((uint8_t)0x40)
#define  ExistLockingTab1							((uint8_t)0x20)
#define  ExistDoorMag1								((uint8_t)0x10)

#define	 LockingTab2Open							((uint8_t)0x08) //副锁舌
#define  DoorMag2Open									((uint8_t)0x04) //
#define	 LockingTab1Open							((uint8_t)0x02) //主锁舌
#define  DoorMag1Open									((uint8_t)0x01) //门磁


#define CityCode_cd 01     //成都市代码5101
#define Citycode_zg 03     //自贡市代码5103



 /** 
  * @brief lock information Structure definition
  */ 
//====================用户平台码================================	 
typedef struct
{
	uint8_t UserPermission;//用户权限码一个字节
	uint8_t CityCode;   //城市代码，1个字节
	uint8_t  *pUserCode ;//UserCode[UserCodeByteSize]={'s','c','t','t',1,6};//用户码：四川铁塔UserCodeByteSize：6个字节
}PlatformCode;

//===================封装开锁秘钥===============================
typedef struct
{
	PlatformCode	 PlatCode;
	const uint8_t   *pLockID;//=&LockIDMemory ;
}LockOpenPassword;


/**
  * @}brief a new station sets up ,device sends message to server.
	* 接收到命令后，执行数据返回状态
  */
typedef struct 
{
	uint8_t CmdOK[4];
	uint8_t CmdExeSuccess[4];
	uint8_t Error[4];
	uint8_t LengthError[4];
}CmdRecvFeedback;//命令接收后处理信息反馈

extern uint8_t Cmd_LockOpenVali_Flag;
extern uint8_t Cmd_LockClosedVali_Flag;


//extern uint8_t PlatformUserCode[UserCodeByteSize];//用户码：四川铁塔UserCodeByteSize：5个字节

extern LockOpenPassword LockPassword;//开锁密钥，包括平台码（其中含：权限码，城市，用户码）以及锁ID

extern _Bool Timer3_10S_Flag;
extern _Bool Timer2_1S_Flag;
extern _Bool Timer2_1S_Beep_Flag;
/** @addtogroup Lock Init functions 
  * @{
  */
void Lock_Init(void);
extern uint8_t ReadDoorLockState(void);
extern uint8_t LockOpenFunc(uint8_t *CmdMemory, uint8_t AddressPoint,uint8_t ByteNum);
extern uint8_t LockClosedFunc(uint8_t *CmdMemory, uint8_t AddressPoint,uint8_t ByteNum);
extern uint8_t ReadDoorLockStaFunc(void);
extern uint8_t NewStationFunc(uint8_t *CmdMemory, uint8_t AddressPoint,uint8_t ByteNum);

extern uint8_t Exe_Close_Lock(void);//上锁执行函数
extern uint8_t Exe_Open_Lock(void);//开锁执行函数

extern uint8_t Mag_Lock_Singal_Out(void);//门锁信号输出
#ifdef __cplusplus
}
#endif

#endif /* __LOCKINFO_H */
 

