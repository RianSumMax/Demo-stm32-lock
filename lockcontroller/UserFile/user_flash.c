#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_gpio.h"
#include "usart.h"
#include "gpio.h"
#include "lockinfo.h"
#include "crc16.h"
#include "user_flash.h"

//算出地址所在的片区首地址
uint32_t GetSector(uint32_t Address)
{
  uint32_t sectorAdr = 0;
  Address = Address - ADDR_FLASH_SECTOR_0;
	sectorAdr = Address/1024;
	sectorAdr = sectorAdr*1024 + ADDR_FLASH_SECTOR_0;
  return sectorAdr;
}

uint16_t Flash_Write_UserCode(uint32_t StarAdr, uint8_t *pUserCode, uint16_t Size)
{
	FLASH_EraseInitTypeDef FEUserC;//初始化状态变量
	uint32_t pPageError = 0;
	uint32_t SectorAdr;
	uint16_t index;
	uint16_t returnVal = 0;
	uint16_t temp;
	uint32_t tempSum = 0;
	uint32_t tempSum2 = 0;
	
	if(HAL_FLASH_Unlock() == HAL_OK)//
	{
		SectorAdr = GetSector(StarAdr);//获取片区地址

		FEUserC.TypeErase = FLASH_TYPEERASE_PAGES;
		FEUserC.PageAddress = SectorAdr;
		FEUserC.NbPages = 1;
		FEUserC.Banks = FLASH_BANK_1;
		HAL_FLASHEx_Erase(&FEUserC,&pPageError);//擦除指定页
		
		for(index = 0; index < Size; index++ )
		{
			tempSum +=  (pUserCode[index] & 0x00ff);
			HAL_FLASH_Program( FLASH_TYPEPROGRAM_HALFWORD, StarAdr + (index *2) , pUserCode[index] & 0x00ff );
		}
		
		HAL_FLASH_Lock();
		
		for(index = 0; index < Size; index++)
		{
			temp = *(__IO uint16_t*)(StarAdr+ (index *2));
			tempSum2 += (temp & 0xff);
		}
		if(tempSum == tempSum2)//存入的数据验证正确
		{
			returnVal = 0x01;
		}
		else
		{
			returnVal = 0x00;
		}
	}
	else
	{
		returnVal = 0x00;
	}
	
	return returnVal;
}

uint16_t Flash_Read_UserCode(uint32_t StarAdr, uint8_t *pUserCode, uint16_t Size)
{
	uint32_t index;
	uint16_t  temp;
	
	for(index = 0; index < Size; index++)
	{
		temp = *(__IO uint16_t*)(StarAdr+ (index *2));
		pUserCode[index] = temp & 0xff;
	}
	return 0x01;
}



