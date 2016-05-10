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
 #ifndef  __USER_FLASH_H
 #define  __USER_FLASH_H
 
 #ifdef __cplusplus
 extern "C" {
#endif 
/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx.h"
#include "Legacy/stm32_hal_legacy.h"
#include <stdio.h>
#include "usart.h"
#include "user_usart.h"
#include "tim.h"
	 
/* Base address of the Flash sectors */
#define ADDR_FLASH_SECTOR_0      ((uint32_t)0x08000000) /* Base @ of Sector 0, 1 Kbytes */
#define ADDR_FLASH_SECTOR_1      ((uint32_t)0x08000400) /* Base @ of Sector 1, 1 Kbytes */
#define ADDR_FLASH_SECTOR_2      ((uint32_t)0x08000800) /* Base @ of Sector 2, 1 Kbytes */
#define ADDR_FLASH_SECTOR_3      ((uint32_t)0x08000C00) /* Base @ of Sector 3, 1 Kbytes */
#define ADDR_FLASH_SECTOR_4      ((uint32_t)0x08001000) /* Base @ of Sector 4, 1 Kbytes */
#define ADDR_FLASH_SECTOR_5      ((uint32_t)0x08001400) /* Base @ of Sector 5, 1 Kbytes */
#define ADDR_FLASH_SECTOR_6      ((uint32_t)0x08001800) /* Base @ of Sector 6, 1 Kbytes */
#define ADDR_FLASH_SECTOR_7      ((uint32_t)0x08001C00) /* Base @ of Sector 7, 1 Kbytes */
#define ADDR_FLASH_SECTOR_8      ((uint32_t)0x08002000) /* Base @ of Sector 8, 1 Kbytes */
#define ADDR_FLASH_SECTOR_9      ((uint32_t)0x08002400) /* Base @ of Sector 9, 1 Kbytes */
#define ADDR_FLASH_SECTOR_10     ((uint32_t)0x08002800) /* Base @ of Sector 10, 1 Kbytes */
#define ADDR_FLASH_SECTOR_11     ((uint32_t)0x08002C00) /* Base @ of Sector 11, 1 Kbytes */
#define ADDR_FLASH_SECTOR_12     ((uint32_t)0x08003000) /* Base @ of Sector 12, 1 Kbytes */
#define ADDR_FLASH_SECTOR_13     ((uint32_t)0x08003400) /* Base @ of Sector 13, 1 Kbytes */
#define ADDR_FLASH_SECTOR_14     ((uint32_t)0x08003800) /* Base @ of Sector 14, 1 Kbytes */
#define ADDR_FLASH_SECTOR_15     ((uint32_t)0x08003C00) /* Base @ of Sector 15, 1 Kbytes */
#define ADDR_FLASH_SECTOR_16     ((uint32_t)0x08004000) /* Base @ of Sector 16, 1 Kbytes */
#define ADDR_FLASH_SECTOR_17     ((uint32_t)0x08004400) /* Base @ of Sector 17, 1 Kbytes */
#define ADDR_FLASH_SECTOR_18     ((uint32_t)0x08004800) /* Base @ of Sector 18, 1 Kbytes */
#define ADDR_FLASH_SECTOR_19     ((uint32_t)0x08004C00) /* Base @ of Sector 19, 1 Kbytes */
#define ADDR_FLASH_SECTOR_20     ((uint32_t)0x08005000) /* Base @ of Sector 20, 1 Kbytes */
#define ADDR_FLASH_SECTOR_21     ((uint32_t)0x08005400) /* Base @ of Sector 21, 1 Kbytes */
#define ADDR_FLASH_SECTOR_22     ((uint32_t)0x08005800) /* Base @ of Sector 22, 1 Kbytes */
#define ADDR_FLASH_SECTOR_23     ((uint32_t)0x08005C00) /* Base @ of Sector 23, 1 Kbytes */
#define ADDR_FLASH_SECTOR_24     ((uint32_t)0x08006000) /* Base @ of Sector 24, 1 Kbytes */
#define ADDR_FLASH_SECTOR_25     ((uint32_t)0x08006400) /* Base @ of Sector 25, 1 Kbytes */
#define ADDR_FLASH_SECTOR_26     ((uint32_t)0x08006800) /* Base @ of Sector 26, 1 Kbytes */
#define ADDR_FLASH_SECTOR_27     ((uint32_t)0x08006C00) /* Base @ of Sector 27, 1 Kbytes */
#define ADDR_FLASH_SECTOR_28     ((uint32_t)0x08007000) /* Base @ of Sector 28, 1 Kbytes */
#define ADDR_FLASH_SECTOR_29     ((uint32_t)0x08007400) /* Base @ of Sector 29, 1 Kbytes */
	 
//用户存放地址
#define Flash_User_StartAddr			ADDR_FLASH_SECTOR_20



extern void FLASH_PageErase(uint32_t PageAddress);

extern uint16_t Flash_Write_UserCode(uint32_t StarAdr, uint8_t *pUserCode, uint16_t Size);
extern uint16_t Flash_Read_UserCode(uint32_t StarAdr, uint8_t *pUserCode, uint16_t Size);
#ifdef __cplusplus
}
#endif

#endif /* __LOCKINFO_H */


