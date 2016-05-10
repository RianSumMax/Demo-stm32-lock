#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_gpio.h"
#include "gpio.h"
#include "lockinfo.h"
#include "crc16.h"
#include "user_flash.h"

const uint8_t LockIDMem[8] = {'L',0x11,0x22,0x33,0x44,0x55,0x66,0x77};//锁ID存储
uint8_t PlatformUserCode[UserCodeByteSize]={'s','c','t','t',0x01,0x06};//用户码：四川铁塔UserCodeByteSize：5个字节

uint8_t Cmd_LockOpenVali_Flag = 0;//开门动作标志位，这位置‘1’，将启动开门驱动程序
uint8_t Cmd_LockClosedVali_Flag = 0;//关门动作标志位，这位置‘1’，将启动关门驱动程序
_Bool Timer2_1S_Beep_Flag = 0;//开门时用到1S定时器控制蜂鸣器响

LockOpenPassword LockPassword;//开锁密钥，包括平台码（其中含：权限码，城市，用户码）以及锁ID

//=========================================读取门锁状态========================================================
 uint8_t ReadDoorLockState(void)
{
	uint8_t PinVal = 0;
	uint8_t DoorLockStatus = 0;//门锁状态初始化
	
	if( HAL_GPIO_ReadPin(Mainlock_GPIO_Port, Mainlock_Pin) == MainlockOpenState ){ //当主锁舌引脚输入高时，表示主锁舌打开
		PinVal |= LockingTab1Open;
	}
	else{
		PinVal &=(~LockingTab1Open);
	}
	if(HAL_GPIO_ReadPin(Afflock_GPIO_Port, Afflock_Pin) == AfflockOpenState){    //当副锁舌引脚输入高时，表示副锁舌打开
		PinVal |= LockingTab2Open;
	}
	else{
		PinVal &=(~LockingTab2Open);
	}
	if( HAL_GPIO_ReadPin(Magdoor_GPIO_Port,Magdoor_Pin) == DoorOpenState){			//门磁输入为高时，表示门磁已打开
		PinVal |= DoorMag1Open;
	}
	else{
		PinVal &=(~DoorMag1Open);
	}
	
	DoorLockStatus = ExistLockingTab2 | ExistLockingTab1 | ExistDoorMag1 |PinVal ;//存在主锁舌、副锁舌和一个门磁
		
	return DoorLockStatus;
}

//=======================初始化==============================================
void Lock_Init(void)
{
	uint8_t PlatformUserCodeTemp[UserCodeByteSize]={'0'};
	uint16_t index,HadCodeFlag = 0;
	ReadDoorLockState();
	
	//======================LED灯灭==========================================
	HAL_GPIO_WritePin(WorkStatusLed_GPIO_Port, WorkStatusLed_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(KeyStatusLed_GPIO_Port, KeyStatusLed_Pin, GPIO_PIN_SET);
	
	//=======================开锁权限设置======================================
	LockPassword.PlatCode.UserPermission = 0;//初始权限码为0
	LockPassword.PlatCode.CityCode = CityCode_cd;//设定为成都市
	LockPassword.PlatCode.pUserCode = PlatformUserCode;//用户码指针指向PlatformUserCode
	
	//========================锁ID设置==========================================
	LockPassword.pLockID = LockIDMem;//锁ID常量指针指向LockIDMemory
	
	//========================读取上次had wrote的用户码==============================
	
	Flash_Read_UserCode(Flash_User_StartAddr, PlatformUserCodeTemp, UserCodeByteSize);//在指定位置读取用户码
	for(index = 0; index < UserCodeByteSize -1; index++ )
	{
		if (PlatformUserCodeTemp[index] == PlatformUserCodeTemp[index + 1])
		{
			if ((PlatformUserCodeTemp[index] == 0) || (PlatformUserCodeTemp[index] == 0xff))
				HadCodeFlag = 0;
			else
			{
				HadCodeFlag = 1;
			}
		}
		else
		{
			HadCodeFlag = 1;
		}
		if (HadCodeFlag)
			break;
	}
	if(HadCodeFlag)//如果上次地址中有数据，不为00或0xff，则读取出来作为平台码的用户码
	{
		for(index = 0; index < UserCodeByteSize; index++ )
		{
			PlatformUserCode[index] = PlatformUserCodeTemp[index];
		}
		LockPassword.PlatCode.pUserCode = PlatformUserCode;//用户码指针指向PlatformUserCode
	}
	//test point
	//while(HAL_UART_Transmit_IT(&huart2, LockPassword.PlatCode.pUserCode, UserCodeByteSize) == HAL_BUSY){};//将存入的数，取出显示

}

//======================串口开锁命令下达====================================
uint8_t LockOpenFunc(uint8_t *CmdMemory, uint8_t AddressPoint,uint8_t ByteNum)
{
	uint8_t  CmdExeStatus = 0x00;
	uint8_t Loop_i = 0;
	uint8_t  PlatCodeAdrTemp, LockIDAdrTemp ;
	PlatCodeAdrTemp = AddressPoint + 2;//地址偏移2
	LockIDAdrTemp = AddressPoint + 20;//地址偏移20
	
	//蜂鸣器响一声
	HAL_GPIO_WritePin(Beep_GPIO_Port,Beep_Pin,GPIO_PIN_SET);//蜂鸣器响一声
	HAL_Delay(200);
	HAL_GPIO_WritePin(Beep_GPIO_Port,Beep_Pin,GPIO_PIN_RESET);//蜂鸣器停止响
	
	//===================checked  lock ID======================================================
	for(Loop_i=0;Loop_i<8;Loop_i++)
	{
		if(LockPassword.pLockID [Loop_i] != CmdMemory[LockIDAdrTemp + Loop_i] )
		{
			CmdExeStatus = 0x02;//ID Error
			return CmdExeStatus;
		}
	}
	//while(HAL_UART_Transmit_IT(&huart2, BackStatus_LockOpen[CmdExeStatus],BackStatusListSize) == HAL_BUSY){};
	if( CmdExeStatus != 0x02)
	{
		//=================平台验证(用户密码验证，用户权限验证，城市码验证)=========================
		for(Loop_i=0;Loop_i<6;Loop_i++)
		{
			if( LockPassword.PlatCode.pUserCode[Loop_i] != CmdMemory[PlatCodeAdrTemp + Loop_i + 2] )//密码验证
			{
				CmdExeStatus = 0x01;//密码 Error
				return CmdExeStatus;
			}
		}
		//while(HAL_UART_Transmit_IT(&huart2, BackStatus_LockOpen[CmdExeStatus],BackStatusListSize) == HAL_BUSY){};
		if( CmdExeStatus != 0x01 )//密码正确
		{
			if( 'A' != CmdMemory[PlatCodeAdrTemp])//用户权限验证
			{
				if( LockPassword.PlatCode.CityCode != CmdMemory[PlatCodeAdrTemp+1])//城市验证
				{
					CmdExeStatus = 0x03;//城市代码 Error
					return CmdExeStatus;
				}
			}
			if(CmdExeStatus != 0x03)
			{
				Cmd_LockOpenVali_Flag = 1;//打开开门标志位
				CmdExeStatus = 0x00;  //命令成功执行
			}
		}
	}
	
	return CmdExeStatus;
}
//======================串口关门命令下达====================================
uint8_t LockClosedFunc(uint8_t *CmdMemory, uint8_t AddressPoint,uint8_t ByteNum)
{
	uint8_t  CmdExeStatus = 0x00;
	uint8_t Loop_i = 0;
	uint8_t  PlatCodeAdrTemp, LockIDAdrTemp ;
	PlatCodeAdrTemp = AddressPoint + 2;//地址偏移2
	LockIDAdrTemp = AddressPoint + 20;//地址偏移20
	
	//蜂鸣器响一声
	HAL_GPIO_WritePin(Beep_GPIO_Port,Beep_Pin,GPIO_PIN_SET);//蜂鸣器响一声
	HAL_Delay(200);
	HAL_GPIO_WritePin(Beep_GPIO_Port,Beep_Pin,GPIO_PIN_RESET);//蜂鸣器停止响
	
	//===================checked  lock ID======================================================
	for(Loop_i=0;Loop_i<8;Loop_i++)
	{
		if( LockPassword.pLockID [Loop_i] != CmdMemory[LockIDAdrTemp + Loop_i] )
		{
			CmdExeStatus = 0x02;//ID Error
			return CmdExeStatus;
		}
	}
	//=================平台验证(用户密码验证，用户权限验证，城市码验证)=========================
	for(Loop_i=0;Loop_i<6;Loop_i++)
	{
		if( LockPassword.PlatCode.pUserCode[Loop_i] != CmdMemory[PlatCodeAdrTemp + Loop_i + 2] )//密码验证
		{
			CmdExeStatus = 0x01;//密码 Error
			return CmdExeStatus;
		}
	}
	if( 'A' != CmdMemory[PlatCodeAdrTemp])//用户权限验证
	{
		if( LockPassword.PlatCode.CityCode != CmdMemory[PlatCodeAdrTemp+1])//城市验证
		{
			CmdExeStatus = 0x02;//城市代码 Error
			return CmdExeStatus;
		}
	}
	
	Cmd_LockClosedVali_Flag = 1;//打开关门标志位
	CmdExeStatus = 0x00;//命令
	//===================确定门已经合上=======================================================
	if(!(ReadDoorLockState()& DoorMag1Open))//门关上
	{
		CmdExeStatus = 0x01;  //命令成功执行
	}
	return CmdExeStatus;
	
}

//======================串口读取门锁状态命令下达============================
uint8_t ReadDoorLockStaFunc(void)
{
	uint8_t  CmdExeStatus = 0x00;
	
  CmdExeStatus = ReadDoorLockState();
	
	return CmdExeStatus;
	
}
//======================串口新建站命令下达====================================
uint8_t NewStationFunc(uint8_t *CmdMemory, uint8_t AddressPoint,uint8_t ByteNum)
{
	uint8_t  CmdExeStatus = 0x00;
	uint8_t Loop_i = 0;
	uint8_t  PlatCodeAdrTemp, LockIDAdrTemp ;
	uint16_t temp = 0;
	
	PlatCodeAdrTemp = AddressPoint + 2;//地址偏移2
	LockIDAdrTemp = AddressPoint + 20;//地址偏移20
	
	//蜂鸣器响一声
	HAL_GPIO_WritePin(Beep_GPIO_Port,Beep_Pin,GPIO_PIN_SET);//蜂鸣器响一声
	HAL_Delay(200);
	HAL_GPIO_WritePin(Beep_GPIO_Port,Beep_Pin,GPIO_PIN_RESET);//蜂鸣器停止响
	
	//===================checked  lock ID======================================================
	for(Loop_i=0;Loop_i<8;Loop_i++)
	{
		if( LockPassword.pLockID[Loop_i] != CmdMemory[LockIDAdrTemp + Loop_i] )
		{
			CmdExeStatus = 0x03;//ID Error	
			return CmdExeStatus;
		}
	}
	if(CmdExeStatus != 0x03)//ID correct
	{
		//=================平台信息写入(用户密码，用户权限，城市码)=========================
		for(Loop_i=0;Loop_i<UserCodeByteSize;Loop_i++)
		{
			 LockPassword.PlatCode.pUserCode[Loop_i] = CmdMemory[PlatCodeAdrTemp +Loop_i + 2]; //写入密码
		}
		LockPassword.PlatCode.UserPermission =  CmdMemory[PlatCodeAdrTemp]; //写入用户权限
		LockPassword.PlatCode.CityCode = CmdMemory[PlatCodeAdrTemp+1];//写入城市码
			
		//将用户密码写入到FLASH中，保存
		temp = Flash_Write_UserCode(Flash_User_StartAddr, LockPassword.PlatCode.pUserCode, UserCodeByteSize);
		
		if(temp ==0x01)//数据写入正确
			CmdExeStatus = 0x01;//返回正确代号
		else
			CmdExeStatus = 0x00;//返回正确代号
	}
	return CmdExeStatus;

}
//======================开锁执行函数====================================
uint8_t Exe_Open_Lock(void)//开锁执行函数
{
	uint32_t FlagVal = 0;
	uint8_t  ReturnVal = 0 ;
	uint8_t  DoorLockState = 0; 
	//uint16_t static FunCount = 0;
	//开门动作，驱动电机
	while((!(ReadDoorLockState()& LockingTab1Open)) && ( FlagVal < 10))//如果主锁舌没打开，关闭，一直等待,最多等待5S
	{
		if(Timer2_1S_Beep_Flag)
		{
			FlagVal++;
		//开灯WorkLed
			HAL_GPIO_TogglePin(WorkStatusLed_GPIO_Port, WorkStatusLed_Pin);//开 WORK  LED
		//蜂鸣器响
			HAL_GPIO_TogglePin(Beep_GPIO_Port,Beep_Pin);
			
			Timer2_1S_Beep_Flag = 0;
		}
		
		//驱动电机，开主锁舌
		HAL_GPIO_WritePin(Dooropen_GPIO_Port,Dooropen_Pin,GPIO_PIN_SET);//开门置位
		HAL_GPIO_WritePin(Doorclose_GPIO_Port,Doorclose_Pin,GPIO_PIN_RESET);//关门引脚复位
	}
	//关灯和关闭蜂鸣器
	HAL_GPIO_WritePin(WorkStatusLed_GPIO_Port,WorkStatusLed_Pin,GPIO_PIN_SET);//
	HAL_GPIO_WritePin(Beep_GPIO_Port,Beep_Pin,GPIO_PIN_RESET);
	//主锁舌打开或者在规定的时间内未打开，停止电机工作
	HAL_GPIO_WritePin(Dooropen_GPIO_Port,Dooropen_Pin,GPIO_PIN_RESET);//
	HAL_GPIO_WritePin(Doorclose_GPIO_Port,Doorclose_Pin,GPIO_PIN_RESET);//
	
	//进入10S倒计时操作环节
	DoorLockState = ReadDoorLockState();
	
	//while(HAL_BUSY == HAL_UART_Transmit_IT(&huart2, GapReqCmd, 2)){};//test
	//while(HAL_BUSY == HAL_UART_Transmit_IT(&huart2, &DoorLockState, 1)){};//test
		
	if((DoorLockState & LockingTab1Open) && (!(DoorLockState & DoorMag1Open))) //主锁舌打开,并且门合上，开启定时器，进入倒计时
	{
	//启动TIMER3，开始中断
		HAL_TIM_Base_Start_IT( &htim3 );//start TIM2 IT and Enable TIM2
   	while(!(ReadDoorLockState() & DoorMag1Open ) && ( ReadDoorLockState() & LockingTab1Open ) && ( FlagVal < 20))//等待门打开,或主锁舌关上
		{
			if(Timer2_1S_Beep_Flag)
			{
				FlagVal++;
			//开灯KeyLed
				HAL_GPIO_TogglePin(KeyStatusLed_GPIO_Port, KeyStatusLed_Pin);//换一个灯开，KEY LED
			//蜂鸣器响
				HAL_GPIO_TogglePin(Beep_GPIO_Port,Beep_Pin);
				
				Timer2_1S_Beep_Flag = 0;
				
	//			while(HAL_BUSY == HAL_UART_Transmit_IT(&huart2, &DoorLockState, 1)){};//test
			}
			if(Timer3_10S_Flag )//10S到关闭主锁舌
			{
				HAL_GPIO_WritePin(Dooropen_GPIO_Port,Dooropen_Pin,GPIO_PIN_RESET);//
				HAL_GPIO_WritePin(Doorclose_GPIO_Port,Doorclose_Pin,GPIO_PIN_SET);//
			}
		}
	}
	//关灯和关闭蜂鸣器
	HAL_GPIO_WritePin(KeyStatusLed_GPIO_Port,KeyStatusLed_Pin,GPIO_PIN_SET);//
	HAL_GPIO_WritePin(Beep_GPIO_Port,Beep_Pin,GPIO_PIN_RESET);
	
	//门磁打开或者主锁舌关闭，停止电机工作
	HAL_GPIO_WritePin(Dooropen_GPIO_Port,Dooropen_Pin,GPIO_PIN_RESET);//
	HAL_GPIO_WritePin(Doorclose_GPIO_Port,Doorclose_Pin,GPIO_PIN_RESET);//
	
	HAL_TIM_Base_Stop_IT( &htim3 );//停止10S定时器
	Timer3_10S_Flag = 0;
	
	DoorLockState = ReadDoorLockState();
	
	if(DoorLockState & DoorMag1Open)
	{
		ReturnVal = 0x01;//开门成功
	}
	else if(!(DoorLockState & LockingTab1Open))//主锁舌合上
	{
		ReturnVal = 0x00;//开门失败
	}
	else 
	{
		ReturnVal = 0x00;//开门失败
	}
		return ReturnVal;
}
//======================上锁执行函数====================================
uint8_t Exe_Close_Lock(void)//上锁执行函数
{
	uint8_t FlagVal = 0;
	uint8_t  DoorLockState = 0; 
	uint8_t ReturnVal = 0;
	
	DoorLockState = ReadDoorLockState();
	if(!(DoorLockState & DoorMag1Open) && (DoorLockState & LockingTab1Open))//门磁合上，主锁舌打开，才能执行上锁动作
	{
		while((ReadDoorLockState()& LockingTab1Open) && ( FlagVal < 5))//等待关门成功
		{
			if(Timer2_1S_Beep_Flag)
			{
				FlagVal++;
			//开灯WorkLed
				HAL_GPIO_TogglePin(KeyStatusLed_GPIO_Port, KeyStatusLed_Pin);//开 WORK  LED
			//蜂鸣器响
				HAL_GPIO_TogglePin(Beep_GPIO_Port,Beep_Pin);
				
				Timer2_1S_Beep_Flag = 0;
			}
			
			//驱动电机，关门
			HAL_GPIO_WritePin(Dooropen_GPIO_Port,Dooropen_Pin,GPIO_PIN_RESET);//
			HAL_GPIO_WritePin(Doorclose_GPIO_Port,Doorclose_Pin,GPIO_PIN_SET);//关门
			
		}
	}
	//关灯和关闭蜂鸣器
	HAL_GPIO_WritePin(KeyStatusLed_GPIO_Port,KeyStatusLed_Pin,GPIO_PIN_SET);// WORK  LED
	HAL_GPIO_WritePin(Beep_GPIO_Port,Beep_Pin,GPIO_PIN_RESET);
	
	//门磁打开或者主锁舌关闭，停止电机工作
	HAL_GPIO_WritePin(Dooropen_GPIO_Port,Dooropen_Pin,GPIO_PIN_RESET);//
	HAL_GPIO_WritePin(Doorclose_GPIO_Port,Doorclose_Pin,GPIO_PIN_RESET);//
	
	DoorLockState = ReadDoorLockState();
	if(!(DoorLockState & DoorMag1Open) && (!(DoorLockState & LockingTab1Open)))
	{
		ReturnVal = 0x01;//关门成功
	}
	else 
	{
		ReturnVal = 0x00;//关门失败
	}
	
	return ReturnVal;
}


uint8_t Mag_Lock_Singal_Out(void)//门锁信号输出
{
	  int8_t ReturnVal = 0;
	//门磁信号输出
	 if(ReadDoorLockState() & DoorMag1Open)//门磁1打开
		{
		  HAL_GPIO_WritePin(MagDoorSingalOut_GPIO_Port,MagDoorSingalOut_Pin,GPIO_PIN_RESET);//输出门磁信号，光耦导通
		}
		else
		{
			HAL_GPIO_WritePin(MagDoorSingalOut_GPIO_Port,MagDoorSingalOut_Pin,GPIO_PIN_SET);//输出门磁信号，光耦开路
		}
	//锁舌信号输出	
		if(ReadDoorLockState() & LockingTab1Open)//主锁舌打开
		{
		  HAL_GPIO_WritePin(LockSingalOut_GPIO_Port,LockSingalOut_Pin,GPIO_PIN_RESET);//输出主锁舌信号，光耦导通
		}
		else
		{
			HAL_GPIO_WritePin(LockSingalOut_GPIO_Port,LockSingalOut_Pin,GPIO_PIN_SET);//输出主锁舌信号，光耦开路
		}	
		ReturnVal = 0x01;
		
	return ReturnVal;
}

