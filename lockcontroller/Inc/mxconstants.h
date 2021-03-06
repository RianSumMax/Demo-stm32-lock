/**
  ******************************************************************************
  * File Name          : mxconstants.h
  * Description        : This file contains the common defines of the application
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

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/
#define TIM2CounterPeriod 1999
#define Timer2_1S 1
#define Timer3_10S 10

#define Beep_Pin GPIO_PIN_1
#define Beep_GPIO_Port GPIOC
#define ElecKeyTX_Pin GPIO_PIN_2
#define ElecKeyTX_GPIO_Port GPIOA
#define ElecKeyRX_Pin GPIO_PIN_3
#define ElecKeyRX_GPIO_Port GPIOA
#define LockSingalOut_Pin GPIO_PIN_4
#define LockSingalOut_GPIO_Port GPIOC
#define MagDoorSingalOut_Pin GPIO_PIN_5
#define MagDoorSingalOut_GPIO_Port GPIOC
#define FsuIn_Pin GPIO_PIN_0
#define FsuIn_GPIO_Port GPIOB
#define Dooropen_Pin GPIO_PIN_14
#define Dooropen_GPIO_Port GPIOB
#define Doorclose_Pin GPIO_PIN_15
#define Doorclose_GPIO_Port GPIOB
#define Magdoor_Pin GPIO_PIN_7
#define Magdoor_GPIO_Port GPIOC
#define Mainlock_Pin GPIO_PIN_8
#define Mainlock_GPIO_Port GPIOC
#define Afflock_Pin GPIO_PIN_9
#define Afflock_GPIO_Port GPIOC
#define KeyStatusLed_Pin GPIO_PIN_11
#define KeyStatusLed_GPIO_Port GPIOA
#define WorkStatusLed_Pin GPIO_PIN_12
#define WorkStatusLed_GPIO_Port GPIOA
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
