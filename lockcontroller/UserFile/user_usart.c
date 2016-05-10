#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_gpio.h"
#include "gpio.h"
#include "lockinfo.h"
#include "crc16.h"
#include "usart.h"
#include "user_usart.h"

	uint8_t USART1_Rec_CmdMemory[USART1_Rec_CmdMemorySize] ={ 0 };
	uint8_t USART1_Rec_CmdMemoryBuff[USART1_Rec_CmdMemoryBuffSize] ={ 0xA1,0xA2,0xA3,0xA0,0x00,0xff };
	uint8_t USART2_Rec_CmdMemory[USART2_Rec_CmdMemorySize] ={ 0 };
	uint8_t USART2_Rec_CmdMemoryBuff[USART2_Rec_CmdMemoryBuffSize] ={ 0xB1,0xB2,0xB3,0xB0,0x00,0xff };	
	uint8_t USART1_Rec_CmdMemory_Count = 0;//存放当下可存储位置
	uint8_t USART2_Rec_CmdMemory_Count = 0;//存放当下可存储位置
	
	uint8_t USART1_Rec_Cmd_Succ_Count = 0;//计数器Count表示：串口1成功接收到命令的条数
	uint8_t USART2_Rec_Cmd_Succ_Count = 0;//计数器Count表示：串口2成功接收到命令的条数
	
	uint8_t USART1_Rec_Cmd_Succ_ByteNum[USART1_Rec_Cmd_Succ_ByteNumSize] = {0};
	//上面表示每条命令所对应的字节数：例如第一条对应的字节数放在 USART1_Rec_Cmd_Succ_ByteNum[0] 中
	uint8_t USART2_Rec_Cmd_Succ_ByteNum[USART2_Rec_Cmd_Succ_ByteNumSize] = {0};
	//上面表示每条命令所对应的字节数：例如第一条对应的字节数放在 USART2_Rec_Cmd_Succ_ByteNum[0] 中
	
	uint8_t GapReqCmd[6] = {0xaa,0x24,0x00,0x01,0xff,0x00};//设备准备接收下一帧数据时，向对方发送间隔请求命令
	
	uint8_t BackStatus_LockOpen[4][BackStatusListSize]=
													{{0x80, 0x7E, LockOpenCommand, LockOpenStatus_Success, 0xA0, 0x85},
													{0x80, 0x7E, LockOpenCommand, LockOpenStatus_Error,   0xA0, 0xD5},
													{0x80, 0x7E, LockOpenCommand, LockOpenStatus_CmdError,0xA1, 0x25},
													{0x80, 0x7E, LockOpenCommand, LOckOpenStatus_PlatError,0xA0, 0xfd},
						                      	};
	uint8_t BackStatus_LockClosed[3][BackStatusListSize]=
													{{0x80, 0x7E, LockClosedCommad, LockClosedStatus_Success, 0x5D, 0x25},
													 {0x80, 0x7E, LockClosedCommad, LockClosedStatus_Error,   0x5C, 0x85},
													 {0x80, 0x7E, LockClosedCommad, LockClosedStatus_CmdError,0x5D, 0x75},
													};
	uint8_t BackStatus_NewStation[4][BackStatusListSize]=
													{{0x80, 0x7E, NewStationCommand, NewStationStatus_Success,    0x98, 0x70},
													 {0x80, 0x7E, NewStationCommand, NewStationStatus_CmdSuccess, 0x99, 0x80},
													 {0x80, 0x7E, NewStationCommand, NewStationStatus_Error,      0x99, 0xD0},
													 {0x80, 0x7E, NewStationCommand, NewStationStatus_CmdError,   0x98, 0x20},
													};
	uint8_t BackStatus_DoorLockState[2][BackStatusListSize]=
													{{0x80, 0x7E, ReadLockStatusCommand, 0x00 ,    0x00, 0x00},
													 {0x80, 0x7E, ReadLockStatusCommand, DoorLockStatus_CmdError , 0x62, 0x98}
													};
	
	//===============================================================================================================
	/**
  * @brief 函数功能：实现将串口缓存数组中一帧数据存入到控制命令存储数组中
  * @param  函数入口参数：*buff : 串口缓存数组
  *                       *CmdMemory :将要存入的数组
	*												*BuffSize : 缓存数组大小
	*												Count ： 记录CmdMemory 缓存位置
  * @retval 函数返回参数：下一次存放位置
  */
uint8_t SavingUSARTxRecCmd_Buff_To_Memory(uint8_t *Buff, uint8_t *CmdMemory, uint8_t BuffSize, uint8_t Count)
{
	uint8_t temp = 0, Loop_i = 0;
	temp = Buff[0];//其实字符判断
	if( temp <= 0x80 )
	{
		for(Loop_i = 0; Loop_i < BuffSize-1; Loop_i++)//去掉第一个标志数据
		{
			CmdMemory[Count+Loop_i] = Buff[Loop_i+1];
		}
		Count = BuffSize -1;//去掉第一个数据，需要减去一
	}
	return Count;
}
	//===============================================================================================================
/**
  * @brief 函数功能：串口2实现一条命令数据存入到控制命令存储数组中
  * @param  函数入口参数：CountTemp 表示某条命令的帧数, 第一帧是以01开头，第二帧以02开头，结束帧：80
	* @global param 全局参数： USART2_Rec_CmdMemory_Count ，记录串口存储区应当存放位置
	* @retval 函数返回参数：返回状态信息 01 表示 此为第一帧，还有下一帧；00表示结束帧。0xff等待下一个扫描周期，本次不操作
	* @reference 函数调用：SavingUSARTxRecCmd_Buff_To_Memory() 
  */
uint8_t SavingUSART2RecCmd_To_Memory(uint8_t CountTemp)
{
	uint8_t FirstTemp, Location_Count;
	uint8_t ReternVal = 0;
	
	FirstTemp = USART2_Rec_CmdMemoryBuff[0];//起始字符判断，每一帧都是有帧标号
	
	Location_Count = USART2_Rec_CmdMemoryBuffSize-1;
	
	//if( FirstTemp <0x80 )
	if( FirstTemp == CountTemp )
	{
		if(Location_Count <= (USART2_Rec_CmdMemorySize - USART2_Rec_CmdMemory_Count) ) //确保不会超出要写入的地址空间
		{
			SavingUSARTxRecCmd_Buff_To_Memory( USART2_Rec_CmdMemoryBuff, USART2_Rec_CmdMemory,
																					USART2_Rec_CmdMemoryBuffSize, USART2_Rec_CmdMemory_Count );
			
			USART2_Rec_CmdMemory_Count += Location_Count;	
			
			ReternVal = 0x01;//空间足够，能够存下
		}
		else
		{
			ReternVal = 0xFF;//空间不够，返回FF，等待下一个周期，不取出缓存。
		}
	}
	else if( FirstTemp == 0x80)
	{
		if(Location_Count <= (USART2_Rec_CmdMemorySize - USART2_Rec_CmdMemory_Count) ) //确保不会超出要写入的地址空间
		{
			SavingUSARTxRecCmd_Buff_To_Memory( USART2_Rec_CmdMemoryBuff, USART2_Rec_CmdMemory,
																					USART2_Rec_CmdMemoryBuffSize, USART2_Rec_CmdMemory_Count );
			
			USART2_Rec_CmdMemory_Count += Location_Count;//改变全局变量，使其指向下一个指令需要存放的位置
			ReternVal = 0x00;//存完最后一帧，返回结束命令
		}
		else
		{
			ReternVal = 0xFF;
		} 
	}
	else
	{
		ReternVal = 0xaa;
	}
	return ReternVal;
}
//===============================================================================================================
/**
  * @brief 函数功能：串口1实现一条命令数据存入到控制命令存储数组中
  * @param  函数入口参数：无
	* @global param 全局参数：USART1_Rec_CmdMemory_Count，记录串口存储区应当存放位置
	* @retval 函数返回参数：返回状态信息 01 表示 此为第一帧，还有下一帧；00表示结束帧。0xff等待下一个扫描周期，本次不操作
	* @reference 函数调用：SavingUSARTxRecCmd_Buff_To_Memory() 
  */
uint8_t SavingUSART1RecCmd_To_Memory(uint8_t CountTemp)
{
	uint8_t FirstTemp, Location_Count;
	uint8_t ReternVal = 0;
	
	FirstTemp = USART1_Rec_CmdMemoryBuff[0];//起始字符判断，每一帧都是有帧标号
	
	Location_Count = USART1_Rec_CmdMemoryBuffSize-1;
	
//if( FirstTemp < 0x80 )
	if( FirstTemp == CountTemp )
	{
		if( Location_Count <= (USART1_Rec_CmdMemorySize - USART1_Rec_CmdMemory_Count) ) //确保不会超出要写入的地址空间
		{
			SavingUSARTxRecCmd_Buff_To_Memory( USART1_Rec_CmdMemoryBuff, USART1_Rec_CmdMemory,
																					USART1_Rec_CmdMemoryBuffSize, USART1_Rec_CmdMemory_Count );
			
			USART1_Rec_CmdMemory_Count += Location_Count;	
			
			ReternVal = 0x01;//空间足够，能够存下
		}
		else
		{
			ReternVal = 0xFF;//空间不够，返回FF，等待下一个周期，不取出缓存。
		}
	}
	else if( FirstTemp == 0x80)
	{
		if(Location_Count <= (USART1_Rec_CmdMemorySize - USART1_Rec_CmdMemory_Count) ) //确保不会超出要写入的地址空间
		{
			SavingUSARTxRecCmd_Buff_To_Memory( USART1_Rec_CmdMemoryBuff, USART1_Rec_CmdMemory,
																					USART1_Rec_CmdMemoryBuffSize, USART1_Rec_CmdMemory_Count );
			
			USART1_Rec_CmdMemory_Count += Location_Count;//改变全局变量，使其指向下一个指令需要存放的位置
			ReternVal = 0x00;//存完最后一帧，返回结束命令
		}
		else
		{
			ReternVal = 0xFF;
		} 
	}
	else
	{
		ReternVal = 0xaa;
	}
	return ReternVal;
}
//===============================================================================================================
/**
  * @brief 函数功能：在串口中断后，完成接收控制命令
  * @param  函数入口参数：无
	* @global param 全局参数： CountTemp2  为静态变量，全局参数，记录存入的次数，以此求出每次存入的字节数
	*													USART2_Rec_Cmd_Succ_ByteNum[] ，存入相应命令的字节数；
	*													USART2_Rec_Cmd_Succ_Count++ ，自增，表示命令条数。
	* @retval 函数返回参数： Rec_Cmd_Succ_Flag ：标志位：指示控制命令接收完成
	* @reference 函数调用： SavingUSART2RecCmd_To_Memory()  
	* @others  备注： 字节数计算方式： CountTemp2 * (USART2_Rec_CmdMemoryBuffSize-1)
  */
BackStatus SavingUSART2RecCmd_To_Memory_Handle(void)
{
	uint8_t USART2_CurRecCmdFramStatus = 0xff;//串口2当前接收命令帧状态，01表示还有下一帧，00表示结束，FF表示暂不操作。
  static uint8_t CountTemp2 =0;			//记录程序进入次数
	BackStatus Rec_Cmd_Succ_Flag = BackNext;
	
	USART2_CurRecCmdFramStatus = SavingUSART2RecCmd_To_Memory( CountTemp2 + 1 );//串口2实现一条命令数据存入到控制命令存储数组中
	
	if(USART2_CurRecCmdFramStatus == 0x01)//表示还有下一帧=======================
	{
		CountTemp2 ++;//存入一帧数据，需要改变计数值
		
		Rec_Cmd_Succ_Flag = BackNext;//命令未完成接收
	}
	else if( USART2_CurRecCmdFramStatus == 0x00) //表示结束帧=====================
	{
		Rec_Cmd_Succ_Flag = BackSuccess;	//控制命令接收完成	
		
		CountTemp2 ++;//存入最后一帧数据，需要改变计数值
		//存下当前命令的字节数
		if(USART2_Rec_Cmd_Succ_Count >= USART2_Rec_Cmd_Succ_ByteNumSize)
		{
			USART2_Rec_Cmd_Succ_Count = 0;
		}
		//存储相应命令对于的字节数
		USART2_Rec_Cmd_Succ_ByteNum[USART2_Rec_Cmd_Succ_Count++] = CountTemp2 * (USART2_Rec_CmdMemoryBuffSize-1);
		
		CountTemp2 = 0;//清零，准备下一条命令的接收
	}
	else if( USART2_CurRecCmdFramStatus == 0xff)
	{
		Rec_Cmd_Succ_Flag = BackWait;//等待下一个回合存入
	}
	else{
		Rec_Cmd_Succ_Flag = BackWrong;
	}
	return Rec_Cmd_Succ_Flag;
}
//===============================================================================================================
/**
  * @brief 函数功能：在串口中断后，完成接收控制命令
  * @param  函数入口参数：无
	* @global param 全局参数： CountTemp1  为静态变量，全局参数，记录存入的次数，以此求出每次存入的字节数；
	*													 USART1_Rec_Cmd_Succ_ByteNum[] ，存入相应命令的字节数；
	*													USART1_Rec_Cmd_Succ_Count++ ，自增，表示命令条数。
	* @retval 函数返回参数： Rec_Cmd_Succ_Flag ：标志位：指示控制命令接收完成
	* @reference 函数调用： SavingUSART1RecCmd_To_Memory()  
	* @others  备注： 字节数计算方式： CountTemp1 * (USART2_Rec_CmdMemoryBuffSize-1)
  */
BackStatus SavingUSART1RecCmd_To_Memory_Handle(void)
{
	uint8_t USART1_CurRecCmdFramStatus = 0xff;//串口1当前接收命令帧状态，01表示还有下一帧，00表示结束，FF表示暂不操作。
  static uint8_t CountTemp1 =0;			//记录程序进入次数
	BackStatus Rec_Cmd_Succ_Flag = BackNext;
	
	USART1_CurRecCmdFramStatus = SavingUSART1RecCmd_To_Memory( CountTemp1+1 );//串口1实现一条命令数据存入到控制命令存储数组中
	
	if(USART1_CurRecCmdFramStatus == 0x01)//表示还有下一帧=====================
	{
		CountTemp1++;			//存入一帧数据，需要改变计数值
		//HAL_UART_Transmit_IT(&huart1, GapReqCmd, 5);//请求对方，发送下一帧数据,在发送完成中断补充接收使能函数
		Rec_Cmd_Succ_Flag = BackNext;//命令未完成接收
	}
	else if( USART1_CurRecCmdFramStatus == 0x00)//表示结束帧=================
	{
		Rec_Cmd_Succ_Flag = BackSuccess;	//控制命令接收完成	
		
		CountTemp1++;   //存入最后一帧数据，需要改变计数值
		//存下当前命令的字节数
		if(USART1_Rec_Cmd_Succ_Count >= USART1_Rec_Cmd_Succ_ByteNumSize)
		{
			USART1_Rec_Cmd_Succ_Count = 0;
		}
		//存储相应命令对于的字节数
		USART1_Rec_Cmd_Succ_ByteNum[USART1_Rec_Cmd_Succ_Count++] = CountTemp1 * (USART1_Rec_CmdMemoryBuffSize-1);
		CountTemp1 = 0;//清零，准备下一次接收
	}
	else if( USART1_CurRecCmdFramStatus == 0xff)
	{
		Rec_Cmd_Succ_Flag = BackWait;//等待下一个回合存入
	}
	else
	{
		Rec_Cmd_Succ_Flag = BackWrong;
	}
	return Rec_Cmd_Succ_Flag;
}

//===============================================================================================================
/**
  * @brief 函数功能：串口2 CRC16校验
  * @param  函数入口参数：USART2的命令存储位置和第几条命令
	* @global param 全局参数：需要使用： USART2_Rec_Cmd_Succ_ByteNum[] ,返回相应命令的字节
	*																	 USART2_Rec_CmdMemory_Count ,返回存储区记录的所有字节数
	* @retval 函数返回参数： Check_MemValiStatus ：标志位：指示校验结果
	* @reference 函数调用：CRC16() 
  */
BackStatus USART2Checked_MemoryVali_Handle(uint8_t *CmdMemory, uint8_t Count)
{
	
	BackStatus Check_MemValiStatus = BackFailure;
	uint16_t Status = 0;
	uint8_t  Loop_i = 0;
	uint8_t  ByteNum = 0;
	uint8_t  AdressPoint = 0;
	
	for(Loop_i=0;Loop_i<Count;Loop_i++)//第一次不需要改变地址
	{
		AdressPoint += USART2_Rec_Cmd_Succ_ByteNum[Loop_i];//获取当前命令的字节数，求和算起始地址
	}
	AdressPoint = AdressPoint- USART2_Rec_Cmd_Succ_ByteNum[0];
	ByteNum = USART2_Rec_Cmd_Succ_ByteNum[Count-1];//获取第Count条指令字节数
	if(Count == 1 && ByteNum != USART2_Rec_CmdMemory_Count)//如果在只有一个命令的情况下，记录的个数有问题
	{
		Status = CRC16(CmdMemory, USART2_Rec_CmdMemory_Count);
	//test
//	HAL_UART_Transmit_IT(&huart2, USART2_Rec_Cmd_Succ_ByteNum, USART2_Rec_Cmd_Succ_Count);//将存入命令相应的字节数打印出来
		
		Check_MemValiStatus = BackWait;
	}
	else
	{
		Status = CRC16(CmdMemory+AdressPoint, ByteNum);
	}
	if( !Status )
	{
		Check_MemValiStatus = BackSuccess;
	}
	
	return Check_MemValiStatus;
}
//===============================================================================================================
/**
  * @brief 函数功能：串口2命令解析
  * @param  函数入口参数：
	* @global param 全局参数：需要使用： USART2_Rec_Cmd_Succ_ByteNum[] ,返回相应命令的字节
	*																	 USART2_Rec_CmdMemory_Count ,返回存储区记录的所有字节数
	* @retval 函数返回参数： Check_MemValiStatus ：标志位：指示校验结果
	* @reference 函数调用： USART2Checked_MemoryVali_Handle(uint8_t *CmdMemory, uint8_t Count) 进行CRC16校验
  */
BackStatus USART2Cmd_Analys_Handle(uint8_t *CmdMemory)
{
	BackStatus HandleStatus = BackFailure;
	uint8_t  index = 0;
	uint8_t  ByteNum = 0; 
	uint8_t  CmdNum  = 0;
	uint8_t  AddressPoint = 0;
	
	CmdNum = USART2_Rec_Cmd_Succ_Count;//获取串口2接收到总命令条数
	
	while(index++ < CmdNum)
	{
		ByteNum = USART2_Rec_Cmd_Succ_ByteNum[index-1];//获取第index条命令的字节数
		
		AddressPoint += ByteNum;
		AddressPoint = AddressPoint - USART2_Rec_Cmd_Succ_ByteNum[0];//计算命令所对应的起始地址
		if(USART2Checked_MemoryVali_Handle(USART2_Rec_CmdMemory, index) == BackSuccess)//CRC16校验通过，返回success，执行命令
		{
			USART2Cmd_Analys_Func(CmdMemory, AddressPoint, ByteNum);//调用命令处理函数，处理第i条命令的字节数
		}
	}
	
	USART2_Rec_Cmd_Succ_Count  = 0;//全局表示命令条数，命令执行完后需要清零
	USART2_Rec_CmdMemory_Count = 0;//全局指向命令存储区中的位置，命令处理完之后需要修改位置信息
	HandleStatus = BackSuccess;
	return HandleStatus;
}
//===============================串口2命令分析函数=================
uint8_t USART2Cmd_Analys_Func(uint8_t *CmdMemory, uint8_t AddressEntry, uint8_t ByteNum)
{
	uint8_t  CmdExeStatus = 0;
	uint8_t  CmdTemp = 0;
	uint16_t CRCValu = 0;
	
	CmdTemp = CmdMemory[AddressEntry + 1];//取出命令字段
	
	switch (CmdTemp)
	{
		case LockOpenCommand: //开门命令
		{
			if(CmdMemory[AddressEntry] == StartComCmdChar){								//起始字符匹配
				CmdExeStatus = LockOpenFunc(CmdMemory,AddressEntry,ByteNum);//命令开门执行函数
			}
			else{
				CmdExeStatus = 0x02;
			}
			while(HAL_UART_Transmit_IT(&huart2, BackStatus_LockOpen[CmdExeStatus],BackStatusListSize) == HAL_BUSY){};
		  break;
		}
		case LockClosedCommad://锁门命令
		{		
			if(CmdMemory[AddressEntry] == StartComCmdChar){								//起始字符匹配
				CmdExeStatus = LockClosedFunc(CmdMemory,AddressEntry,ByteNum);//命令关门执行函数
			}
			else{
				CmdExeStatus = 0x02;
			}
			while(HAL_UART_Transmit_IT(&huart2, BackStatus_LockClosed[CmdExeStatus],BackStatusListSize) == HAL_BUSY){};
		  break;
		}
		case ReadLockStatusCommand: //读取门锁状态命令
		{
			if(CmdMemory[AddressEntry] == StartComCmdChar){								//起始字符匹配
				
				CmdExeStatus = ReadDoorLockStaFunc();//命令关门执行函数
				BackStatus_DoorLockState[0][3] = CmdExeStatus;//写入门锁信号
				CRCValu = CRC16(&BackStatus_DoorLockState[0][1],3);//计算CRC16校验码
				BackStatus_DoorLockState[0][4] = CRCValu/256;//分别放在倒数第二、倒数第一的位置
				BackStatus_DoorLockState[0][5] = CRCValu%256;	
				while(HAL_UART_Transmit_IT(&huart2, BackStatus_DoorLockState[0],BackStatusListSize) == HAL_BUSY){};//返回门锁状态
			}
			else{
				CmdExeStatus = 0x01;
				while(HAL_UART_Transmit_IT(&huart2, BackStatus_DoorLockState[CmdExeStatus],BackStatusListSize) == HAL_BUSY){};
			}
			
			break;
		}
		case NewStationCommand: //新建站命令
		{
			if(CmdMemory[AddressEntry] == StartComCmdChar){								//起始字符匹配
				CmdExeStatus = NewStationFunc(CmdMemory,AddressEntry,ByteNum);//命令新建站执行函数
				if(CmdExeStatus == 0x01){
					while(HAL_UART_Transmit_IT(&huart2, BackStatus_NewStation[0],BackStatusListSize) == HAL_BUSY){};//命令成功
					while(HAL_UART_Transmit_IT(&huart2, BackStatus_NewStation[1],BackStatusListSize) == HAL_BUSY){};//成功执行
				}
				else{
					while(HAL_UART_Transmit_IT(&huart2, BackStatus_NewStation[CmdExeStatus],BackStatusListSize) == HAL_BUSY){};
				}
			}
			else{
					CmdExeStatus = 0x02;
					while(HAL_UART_Transmit_IT(&huart2, BackStatus_NewStation[CmdExeStatus],BackStatusListSize) == HAL_BUSY){};
			}
			break;
		}
		default: ;
	}
	
	return 1;
}
//===============================================================================================================
/**
  * @brief 函数功能：串口1 CRC16校验
  * @param  函数入口参数：USART1的命令存储位置和第几条命令
	* @global param 全局参数：需要使用： USART1_Rec_Cmd_Succ_ByteNum[] ,返回相应命令的字节
	*																	 USART1_Rec_CmdMemory_Count ,返回存储区记录的所有字节数
	* @retval 函数返回参数： Check_MemValiStatus ：标志位：指示校验结果
	* @reference 函数调用：CRC16() 
  */
BackStatus USART1Checked_MemoryVali_Handle(uint8_t *CmdMemory, uint8_t Count)
{
	
	BackStatus Check_MemValiStatus = BackFailure;
	uint16_t Status = 0;
	uint8_t  Loop_i = 0;
	uint8_t  ByteNum = 0;
	uint8_t  AdressPoint = 0;
	
	for(Loop_i=0;Loop_i<Count;Loop_i++)//第一次不需要改变地址
	{
		AdressPoint += USART1_Rec_Cmd_Succ_ByteNum[Loop_i];//获取当前命令的字节数，求和算起始地址
	}
	AdressPoint = AdressPoint- USART1_Rec_Cmd_Succ_ByteNum[0];
	ByteNum = USART1_Rec_Cmd_Succ_ByteNum[Count-1];//获取第Count条指令字节数
	if(Count == 1 && ByteNum != USART1_Rec_CmdMemory_Count)//如果在只有一个命令的情况下，记录的个数有问题
	{
		Status = CRC16(CmdMemory, USART1_Rec_CmdMemory_Count);
	//test
//	HAL_UART_Transmit_IT(&huart2, USART2_Rec_Cmd_Succ_ByteNum, USART2_Rec_Cmd_Succ_Count);//将存入命令相应的字节数打印出来
		
		Check_MemValiStatus = BackWait;
	}
	else
	{
		Status = CRC16(CmdMemory+AdressPoint, ByteNum);
	}
	if( !Status )
	{
		Check_MemValiStatus = BackSuccess;
	}
	
	return Check_MemValiStatus;
}
//===============================================================================================================
/**
  * @brief 函数功能：串口2命令解析
  * @param  函数入口参数：
	* @global param 全局参数：需要使用： USART2_Rec_Cmd_Succ_ByteNum[] ,返回相应命令的字节
	*																	 USART2_Rec_CmdMemory_Count ,返回存储区记录的所有字节数
	* @retval 函数返回参数： Check_MemValiStatus ：标志位：指示校验结果
	* @reference 函数调用： USART2Checked_MemoryVali_Handle(uint8_t *CmdMemory, uint8_t Count) 进行CRC16校验
  */
BackStatus USART1Cmd_Analys_Handle(uint8_t *CmdMemory)
{
	BackStatus HandleStatus = BackFailure;
	uint8_t  index = 0;
	uint8_t  ByteNum = 0; 
	uint8_t  CmdNum  = 0;
	uint8_t  AddressPoint = 0;
	
	CmdNum = USART1_Rec_Cmd_Succ_Count;//获取串口2接收到总命令条数
	
	while(index++ < CmdNum)
	{
		ByteNum = USART1_Rec_Cmd_Succ_ByteNum[index-1];//获取第index条命令的字节数
		
		AddressPoint += ByteNum;
		AddressPoint = AddressPoint - USART1_Rec_Cmd_Succ_ByteNum[0];//计算命令所对应的起始地址
		if(USART1Checked_MemoryVali_Handle(USART1_Rec_CmdMemory, index) == BackSuccess)//CRC16校验通过，返回success，执行命令
		{
			USART1Cmd_Analys_Func(CmdMemory, AddressPoint, ByteNum);//调用命令处理函数，处理第i条命令的字节数
		}
	}
	
	USART1_Rec_Cmd_Succ_Count  = 0;//全局表示命令条数，命令执行完后需要清零
	USART1_Rec_CmdMemory_Count = 0;//全局指向命令存储区中的位置，命令处理完之后需要修改位置信息
	HandleStatus = BackSuccess;
	return HandleStatus;
}

//===============================串口1命令分析函数=================
uint8_t USART1Cmd_Analys_Func(uint8_t *CmdMemory, uint8_t AddressEntry, uint8_t ByteNum)
{
	uint8_t  CmdExeStatus = 0;
	uint8_t  CmdTemp = 0;
	uint16_t CRCValu = 0;
	
	CmdTemp = CmdMemory[AddressEntry + 1];//取出命令字段
	
	switch (CmdTemp)
	{
		case LockOpenCommand: //开门命令
		{
			if(CmdMemory[AddressEntry] == StartComCmdChar){								//起始字符匹配
				CmdExeStatus = LockOpenFunc(CmdMemory,AddressEntry,ByteNum);//命令开门执行函数
			}
			else{
				CmdExeStatus = 0x02;
			}
			while(HAL_UART_Transmit_IT(&huart1, BackStatus_LockOpen[CmdExeStatus],BackStatusListSize) == HAL_BUSY){};
		  break;
		}
		case LockClosedCommad://锁门命令
		{		
			if(CmdMemory[AddressEntry] == StartComCmdChar){								//起始字符匹配
				CmdExeStatus = LockClosedFunc(CmdMemory,AddressEntry,ByteNum);//命令关门执行函数
			}
			else{
				CmdExeStatus = 0x02;
			}
			while(HAL_UART_Transmit_IT(&huart1, BackStatus_LockClosed[CmdExeStatus],BackStatusListSize) == HAL_BUSY){};
		  break;
		}
		case ReadLockStatusCommand: //读取门锁状态命令
		{
			if(CmdMemory[AddressEntry] == StartComCmdChar){								//起始字符匹配
				
				CmdExeStatus = ReadDoorLockStaFunc();//命令关门执行函数
				BackStatus_DoorLockState[0][3] = CmdExeStatus;//写入门锁信号
				CRCValu = CRC16(&BackStatus_DoorLockState[0][1],3);//计算CRC16校验码
				BackStatus_DoorLockState[0][4] = CRCValu/256;//分别放在倒数第二、倒数第一的位置
				BackStatus_DoorLockState[0][5] = CRCValu%256;	
				while(HAL_UART_Transmit_IT(&huart1, BackStatus_DoorLockState[0],BackStatusListSize) == HAL_BUSY){};//返回门锁状态
			}
			else{
				CmdExeStatus = 0x01;
				while(HAL_UART_Transmit_IT(&huart1, BackStatus_DoorLockState[CmdExeStatus],BackStatusListSize) == HAL_BUSY){};
			}
			
			break;
		}
		case NewStationCommand: //新建站命令
		{
			if(CmdMemory[AddressEntry] == StartComCmdChar){								//起始字符匹配
				CmdExeStatus = NewStationFunc(CmdMemory,AddressEntry,ByteNum);//命令新建站执行函数
				if(CmdExeStatus == 0x01){
					while(HAL_UART_Transmit_IT(&huart1, BackStatus_NewStation[0],BackStatusListSize) == HAL_BUSY){};//命令成功
					HAL_Delay(10);
					while(HAL_UART_Transmit_IT(&huart1, BackStatus_NewStation[1],BackStatusListSize) == HAL_BUSY){};//成功执行
				}
				else{
					while(HAL_UART_Transmit_IT(&huart1, BackStatus_NewStation[CmdExeStatus],BackStatusListSize) == HAL_BUSY){};
				}
			}
			else{
					CmdExeStatus = 0x02;
					while(HAL_UART_Transmit_IT(&huart1, BackStatus_NewStation[CmdExeStatus],BackStatusListSize) == HAL_BUSY){};
			}
			break;
		}
		default: ;
	}
	
	return 1;
}



