/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  *
  * COPYRIGHT(c) 2016 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */
#include "lockinfo.h"
#include "user_usart.h"
#include "crc16.h"
#include "user_flash.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
#define aRxBufferSize			20

	_Bool FSUComSigFlag = 0;       //FSU Command signal Flag,FSU 命令信号
  _Bool FSUComSigValidFlag = 0;  //FSU Command signal Validate Flag,FSU 命令信号有效标志位
	

	
	uint8_t aTxBuffer[aRxBufferSize] = {0xe8,0x01,0xa2,03,0xb4,07,9,10,11,00,01,07};
	
	uint8_t aRxBuffer[aRxBufferSize];
  _Bool   aRxFlag = 0;
	
	_Bool Timer2_1S_Flag = 0;
	_Bool Timer2_1S_CheckDoor_Flag = 0;
	_Bool Timer3_10S_Flag = 0;
	uint8_t USART1_RecSuc_Flag = 0;//串口1接收一帧完成标志位
  uint8_t USART2_RecSuc_Flag = 0;//串口2接收一帧完成标志位
	uint8_t USART1_SedSuc_Flag = 0;//串口1接收一帧完成标志位
  uint8_t USART2_SedSuc_Flag = 0;//串口2接收一帧完成标志位
	//uint8_t tempval = 0;
	
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_NVIC_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */
  uint8_t Enter1_Test = 0;
	uint8_t Enter2_Test = 0;
	uint8_t Count_Test = 0;
//	uint8_t tempval = 0;
	
	uint8_t PlatformUserCodeTemp[UserCodeByteSize]={'s','c','t','t',0x01,0x06};//test
	BackStatus Rec_Cmd_Succ_Flag = BackWait;//初始化读入串口指令的程序
	
	//uint8_t beep_count = 0;//蜂鸣器响的延迟
	uint8_t DoorLockStatus = 0;//门锁状态初始化
  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_TIM3_Init();

  /* Initialize interrupts */
  MX_NVIC_Init();

  /* USER CODE BEGIN 2 */
  Lock_Init();//锁初始化，初始相关特性
	
	
	//=======================启动定时器2开启中断1S============================
  HAL_TIM_Base_Start_IT( &htim2 );//start TIM2 IT and Enable TIM2
	
	//=======================串口1发送中断启动发送，接收中断启动==============
	//使能串口1接收中断
	HAL_UART_Receive_IT(&huart1, USART1_Rec_CmdMemoryBuff,USART1_Rec_CmdMemoryBuffSize);
	//使能串口2发送和接收中断，并发送初始状态
	//HAL_UART_Transmit_IT(&huart2, USART2_Rec_CmdMemoryBuff, USART2_Rec_CmdMemoryBuffSize);
	HAL_UART_Receive_IT(&huart2, USART2_Rec_CmdMemoryBuff,USART2_Rec_CmdMemoryBuffSize);
	
	//============================Flash密码写入读出测试===============================
	//test
	Flash_Read_UserCode(Flash_User_StartAddr, PlatformUserCodeTemp, UserCodeByteSize);
	//while(HAL_UART_Transmit_IT(&huart2, PlatformUserCodeTemp, UserCodeByteSize) == HAL_BUSY){};//将存入的数，取出显示
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		
  /* USER CODE END WHILE */
  /* USER CODE BEGIN 3 */
//===========================用户代码开始===============================
		/*******************串口2接收命令处理*********************************/
		if(USART2_RecSuc_Flag)//串口2接收到一帧数据
		{
		//	Enter2_Test = 1;//测试点
			USART2_RecSuc_Flag = 0;//标志位清零
			
			Rec_Cmd_Succ_Flag = SavingUSART2RecCmd_To_Memory_Handle();
			
			if(Rec_Cmd_Succ_Flag == BackSuccess)//成功存储一个命令
			{
			//分析命令帧内容，执行相关命令	
				USART2Cmd_Analys_Handle(USART2_Rec_CmdMemory);	
			}
			else if(Rec_Cmd_Succ_Flag == BackNext)//命令还有下一帧
			{
				//使能准备接收，开始存放在USART2_Rec_CmdMemoryBuff
				//while(HAL_BUSY == HAL_UART_Receive_IT(&huart2, USART2_Rec_CmdMemoryBuff,USART2_Rec_CmdMemoryBuffSize)){};
				
				//请求对方，发送下一帧数据,在发送完成中断补充接收使能函数
				while(HAL_BUSY == HAL_UART_Transmit_IT(&huart2, GapReqCmd, 6)){};
			}
			else if(Rec_Cmd_Succ_Flag == BackWrong)
			{	
				
			}
			else//Waiting mode
			{
				Enter2_Test = 1;//work led Toggle
			}	
		}
		else
		{
		}
		
		/*******************开门命令处理*********************************/
	
		if(Cmd_LockOpenVali_Flag == 0x01 || FSUComSigFlag ==0x01 )//开门命令有效位为1
		{
			Cmd_LockOpenVali_Flag = 0;//清除标志位
			FSUComSigFlag = 0;
			Exe_Open_Lock();//开门
			//while(HAL_BUSY == HAL_UART_Transmit_IT(&huart2, GapReqCmd, 2)){};//test		
			Mag_Lock_Singal_Out();//门锁信号输出
			
		}
		/*******************关门命令处理*********************************/
		else if(Cmd_LockClosedVali_Flag)//关门命令有效位为1
		{
			Cmd_LockClosedVali_Flag = 0;
			Exe_Close_Lock();//关门
			Mag_Lock_Singal_Out();//门锁信号输出
		}
		else
		{
			Mag_Lock_Singal_Out();//门锁信号输出
		}

		/******************检查门是否已关好*******************************/
		
		if(Timer2_1S_CheckDoor_Flag)
		{
			Timer2_1S_CheckDoor_Flag = 0;
			
			//HAL_GPIO_TogglePin(WorkStatusLed_GPIO_Port, WorkStatusLed_Pin);//test
			
			DoorLockStatus = ReadDoorLockState();

			//门磁处于打开状态 或者 门已合上，但是未上锁，需要警示
//			if(((DoorLockStatus & DoorMag1Open) == DoorMag1Open ) || ( (DoorLockStatus & LockingTab1Open) && ( !(DoorLockStatus & DoorMag1Open) ) ))
			if( (DoorLockStatus & LockingTab1Open) && ( !(DoorLockStatus & DoorMag1Open) ) )
			{
	//			HAL_GPIO_TogglePin(KeyStatusLed_GPIO_Port,KeyStatusLed_Pin);//灯亮灭
				Enter1_Test = 1;//驱动蜂鸣器发出响声
			}
			else
			{
				Enter1_Test = 0;
				//HAL_GPIO_WritePin(KeyStatusLed_GPIO_Port,KeyStatusLed_Pin,GPIO_PIN_SET);// LED
				//HAL_GPIO_WritePin(Beep_GPIO_Port,Beep_Pin,GPIO_PIN_RESET);
			}
		}
		
		/******************串口1接收命令处理********************************************/
		if(USART1_RecSuc_Flag)
		{
			//test
			HAL_UART_Transmit_IT(&huart2, USART1_Rec_CmdMemoryBuff, USART1_Rec_CmdMemoryBuffSize);//用串口2把串口1中的数据显示出来
			
			USART1_RecSuc_Flag = 0;//标志位清零
			//存储相关命令帧，如果按照01开头 80结尾会执行存储否则丢弃
			Rec_Cmd_Succ_Flag = SavingUSART1RecCmd_To_Memory_Handle();
			
			if(Rec_Cmd_Succ_Flag == BackSuccess)//成功存储一个命令
			{
			//分析命令帧内容，执行相关命令	
				USART1Cmd_Analys_Handle(USART1_Rec_CmdMemory);	
			}
			else if(Rec_Cmd_Succ_Flag == BackNext)//命令还有下一帧
			{
				//使能准备接收，开始存放在USART1_Rec_CmdMemoryBuff
				//while(HAL_BUSY == HAL_UART_Receive_IT(&huart1, USART1_Rec_CmdMemoryBuff,USART1_Rec_CmdMemoryBuffSize)){};
				
				//请求对方，发送下一帧数据,在发送完成中断补充接收使能函数
				while(HAL_BUSY == HAL_UART_Transmit_IT(&huart1, GapReqCmd, 6)){};
			}
			else if(Rec_Cmd_Succ_Flag == BackWrong)	//错误帧
			{	
				
			}
			else//Waiting mode
			{
				//Enter2_Test = 1;//work led Toggle
			}		
		}
		else
		{
		}
		
		if( Timer2_1S_Flag )
		{
			HAL_GPIO_TogglePin(KeyStatusLed_GPIO_Port,KeyStatusLed_Pin);//灯亮灭
			
			if( Enter1_Test==1 )
				Count_Test++;
			if((Count_Test == 1) && Enter1_Test)
			{
				HAL_GPIO_WritePin(Beep_GPIO_Port,Beep_Pin,GPIO_PIN_SET);			//响1S中，等1S后立即复位蜂鸣器
			}
			else 
			{
				HAL_GPIO_WritePin(Beep_GPIO_Port,Beep_Pin,GPIO_PIN_RESET);
			}
			if(Count_Test > 4)
			{	
					Count_Test = 0;
			}
			
			if( Enter2_Test==1) HAL_GPIO_TogglePin(WorkStatusLed_GPIO_Port, WorkStatusLed_Pin);//指示发送完成
			Timer2_1S_Flag = 0;//标志位清零
		}
		
	//===========================用户代码结束==================================
  }
  /* USER CODE END 3 */

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);

  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/** NVIC Configuration
*/
void MX_NVIC_Init(void)
{
  /* EXTI0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 1);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}

/* USER CODE BEGIN 4 */
//====================FSU 命令下达回调函数˽========================
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	FSUComSigFlag = 1;//FSU signal input pin - Opening command 
	
}
//====================1S定时器TIMER2中断回调函数˽========================
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM2 )//1S定时器TIMER2中断函数
	{
		Timer2_1S_Flag = 1;
		Timer2_1S_Beep_Flag = 1;
		Timer2_1S_CheckDoor_Flag = 1;
		//HAL_GPIO_TogglePin(Beep_GPIO_Port, Beep_Pin); // Output pin reverses when timer2 counted to 1S 
		//HAL_GPIO_TogglePin(KeyStatusLed_GPIO_Port,KeyStatusLed_Pin);//灯亮灭
	}
	else if(htim->Instance == TIM3)//10S定时器回调函数
	{
		Timer3_10S_Flag = 1;
	}
}
//====================串口1和2中断回调函数==========================
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *UartHandle)
{
  USART2_SedSuc_Flag = 1;
//HAL_GPIO_TogglePin(WorkStatusLed_GPIO_Port, WorkStatusLed_Pin);//指示发送完成
// HAL_GPIO_WritePin(WorkStatusLed_GPIO_Port, WorkStatusLed_Pin, GPIO_PIN_SET);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
//	HAL_GPIO_WritePin(Beep_GPIO_Port,Beep_Pin,GPIO_PIN_RESET);
//	HAL_GPIO_TogglePin(Beep_GPIO_Port, Beep_Pin);//指示接收完成
	if(huart->Instance == USART1 )
	{
		USART1_RecSuc_Flag = 1;//表示完成接收一帧数据
		HAL_UART_Receive_IT(&huart1, USART1_Rec_CmdMemoryBuff,USART1_Rec_CmdMemoryBuffSize);//使能准备接收
	}
	else if(huart->Instance == USART2)
	{
		USART2_RecSuc_Flag = 1;//表示完成接收一帧数据
		HAL_UART_Receive_IT(&huart2, USART2_Rec_CmdMemoryBuff,USART2_Rec_CmdMemoryBuffSize);//使能准备接收
	}
	else
	{
	}
	
}
/* USER CODE END 4 */

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
