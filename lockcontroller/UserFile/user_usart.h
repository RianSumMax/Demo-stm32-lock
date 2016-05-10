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
 #ifndef __USER_USART_H
 #define __USER_USART_H
 
 #ifdef __cplusplus
 extern "C" {
#endif 
/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx.h"
#include "Legacy/stm32_hal_legacy.h"
#include <stdio.h>

#define USART1_Rec_CmdMemorySize  100
#define USART1_Rec_CmdMemoryBuffSize 20//一帧数据20字节
#define USART2_Rec_CmdMemorySize  100
#define USART2_Rec_CmdMemoryBuffSize 20
#define USART1_Sed_CmdSize 5
#define USART2_Sed_CmdSize 5
#define USART1_Rec_Cmd_Succ_ByteNumSize 6
#define USART2_Rec_Cmd_Succ_ByteNumSize 6

#define BackStatusListSize		6	 
#define LockOpenCommand  						((uint8_t)0x42) //开锁命令
#define LockClosedCommad 						((uint8_t)0x13) //锁门命令字段或者离站锁门字段
#define ReadLockStatusCommand 			((uint8_t)0x44) //读取锁状态命令
#define NewStationCommand  					((uint8_t)0x1e) //新建一个基站设备命令	 
	 
#define StartComCmdChar   					((uint8_t)0x7E) //通信状态下，每条命令起始字符
	 
	 
	 
typedef enum 
{
  BackFailure =0,
	BackNext = BackFailure, 
  BackSuccess = !BackFailure,
	BackWait		= 2,
	BackWrong		= 3,
} BackStatus;	 

extern	uint8_t USART1_Rec_CmdMemory[USART1_Rec_CmdMemorySize];
extern	uint8_t USART1_Rec_CmdMemoryBuff[USART1_Rec_CmdMemoryBuffSize] ;
	 
extern	uint8_t USART2_Rec_CmdMemory[USART2_Rec_CmdMemorySize] ;
extern	uint8_t USART2_Rec_CmdMemoryBuff[USART2_Rec_CmdMemoryBuffSize] ;	

extern	uint8_t GapReqCmd[6];//间隔发送字段，获取下一帧

extern	uint8_t USART1_Rec_CmdMemory_Count;//存放当下可存储位置
extern	uint8_t USART2_Rec_CmdMemory_Count;//存放当下可存储位置

extern  uint8_t USART1_Rec_Cmd_Succ_Count;//计数器Count表示：串口1成功接收到命令的条数
extern  uint8_t USART2_Rec_Cmd_Succ_Count;//计数器Count表示：串口2成功接收到命令的条数

extern	uint8_t USART1_Rec_Cmd_Succ_ByteNum[USART1_Rec_Cmd_Succ_ByteNumSize];
	//上面表示每条命令所对应的字节数：例如第一条对应的字节数放在 USART1_Rec_Cmd_Succ_ByteNum[0] 中
extern	uint8_t USART2_Rec_Cmd_Succ_ByteNum[USART2_Rec_Cmd_Succ_ByteNumSize];
	//上面表示每条命令所对应的字节数：例如第一条对应的字节数放在 USART2_Rec_Cmd_Succ_ByteNum[0] 中

extern uint8_t BackStatus_LockOpen[4][BackStatusListSize];
extern uint8_t BackStatus_LockClosed[3][BackStatusListSize];
extern uint8_t BackStatus_NewStation[4][BackStatusListSize];
extern uint8_t BackStatus_DoorLockState[2][BackStatusListSize];//状态信息返回存放数组

/** @addtogroup Storage functions 
  * @{
  */
extern uint8_t SavingUSART2RecCmd_To_Memory(uint8_t CountTemp);
extern uint8_t SavingUSART1RecCmd_To_Memory(uint8_t CountTemp);

extern BackStatus SavingUSART2RecCmd_To_Memory_Handle(void);
extern BackStatus SavingUSART1RecCmd_To_Memory_Handle(void);

extern BackStatus USART2Checked_MemoryVali_Handle(uint8_t *CmdMemory, uint8_t Count);
extern BackStatus USART2Cmd_Analys_Handle(uint8_t *CmdMemory);
extern uint8_t USART2Cmd_Analys_Func(uint8_t *CmdMemory, uint8_t AdressPoint, uint8_t ByteNum);
//===============================串口1命令分析函数=================
uint8_t USART1Cmd_Analys_Func(uint8_t *CmdMemory, uint8_t AddressEntry, uint8_t ByteNum);
extern BackStatus USART1Cmd_Analys_Handle(uint8_t *CmdMemory);//串口2命令解析

#ifdef __cplusplus
}
#endif

#endif /* __LOCKINFO_H */	 


