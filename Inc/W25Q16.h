#ifndef W25Q16_H_
#define W25Q16_H_

#include "main.h"
#include "stm32f4xx_hal.h"

#define SPI_FLASH_PageSize      256
#define SPI_FLASH_PerWritePageSize      256

#define SPI_FLASH_CS_LOW()       HAL_GPIO_WritePin(F_CS_GPIO_Port, F_CS_Pin, 0)
#define SPI_FLASH_CS_HIGH()      HAL_GPIO_WritePin(F_CS_GPIO_Port, F_CS_Pin, 1)

/* Private define ------------------------------------------------------------*/
#define W25X_WriteEnable		      0x06
#define W25X_WriteDisable		      0x04
#define W25X_ReadStatusReg		    0x05
#define W25X_WriteStatusReg		    0x01
#define W25X_ReadData			        0x03
#define W25X_FastReadData		      0x0B
#define W25X_FastReadDual		      0x3B
#define W25X_PageProgram		      0x02
#define W25X_BlockErase			      0xD8
#define W25X_SectorErase		      0x20
#define W25X_ChipErase			      0xC7
#define W25X_PowerDown			      0xB9
#define W25X_ReleasePowerDown	    0xAB
#define W25X_DeviceID			        0xAB
#define W25X_ManufactDeviceID   	0x90
#define W25X_JedecDeviceID		    0x9F

#define WIP_Flag                  0x01  /* Write In Progress (WIP) flag */

#define Dummy_Byte                0xFF

uint32_t W25Q16_ReadID(SPI_HandleTypeDef* hspi);
uint32_t W25Q16_ReadDeviceID(SPI_HandleTypeDef* hspi);
void W25Q16_PowerDown(SPI_HandleTypeDef* hspi);
void W25Q16_WakeUp(SPI_HandleTypeDef* hspi);
void W25Q16_ChipErase(SPI_HandleTypeDef* hspi);
void W25Q16_SectorErase(SPI_HandleTypeDef* hspi, uint32_t SectorAddr);
void W25Q16_BufferWrite(SPI_HandleTypeDef* hspi, uint8_t* pBuffer, uint32_t WriteAddr, uint16_t size);
void W25Q16_BufferRead(SPI_HandleTypeDef* hspi, uint8_t* pBuffer, uint32_t ReadAddr, uint16_t size);

#endif /* W25Q16_H_ */
