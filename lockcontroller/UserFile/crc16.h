/**
  ******************************************************************************
  * @file    CRC16.h
  * @author  CSIS Team
  * @version V1.0.3
  * @date    17-April-2016
  * @brief   Header file of CRC16 module.
  ******************************************************************************
  **/
/* Define to prevent recursive inclusion -------------------------------------*/	
 #ifndef __CRC16_H
 #define __CRC16_H
 #ifdef __cplusplus
 extern "C" {
#endif
	 
/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx.h"
#include "Legacy/stm32_hal_legacy.h"
#include <stdio.h>

typedef union _CRC
{
    uint16_t crc16;
    uint8_t  by[2];
}CRC_Cacu;

extern uint16_t CRC16(uint8_t *ba, int size);

/**
  * @}
  */
#ifdef __cplusplus
}
#endif

#endif /* __LOCKINFO_H */
