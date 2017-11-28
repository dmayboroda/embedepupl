#include "W25Q16.h"

/*******************************************************************************
 * Function Name  : SPI_FLASH_WaitForWriteEnd
 * Description    : Polls the status of the Write In Progress (WIP) flag in the
 *                  FLASH's status  register  and  loop  until write  opertaion
 *                  has completed.
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
static void SPI_FLASH_WaitForWriteEnd(SPI_HandleTypeDef* hspi) {
	uint8_t buffer[] = { W25X_ReadStatusReg };

	/* Select the FLASH: Chip Select low */
	SPI_FLASH_CS_LOW();

	/* Send "Read Status Register" instruction */
	HAL_SPI_Transmit(hspi, buffer, 1, 100);

	/* Loop as long as the memory is busy with a write cycle */
	do {
		/* Send a dummy byte to generate the clock needed by the FLASH
		 and put the value of the status register in FLASH_Status variable */
		buffer[0] = Dummy_Byte;
		HAL_SPI_TransmitReceive(hspi, buffer, buffer, 1, 100);
	} while ((buffer[0] & WIP_Flag) == SET); /* Write in progress */

	/* Deselect the FLASH: Chip Select high */
	SPI_FLASH_CS_HIGH();
}

/*******************************************************************************
 * Function Name  : SPI_FLASH_WriteEnable
 * Description    : Enables the write access to the FLASH.
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
static void SPI_FLASH_WriteEnable(SPI_HandleTypeDef* hspi) {
	uint8_t buffer[] = { W25X_WriteEnable };
	/* Select the FLASH: Chip Select low */
	SPI_FLASH_CS_LOW();

	/* Send "Write Enable" instruction */
	HAL_SPI_Transmit(hspi, buffer, sizeof(buffer), 100);

	/* Deselect the FLASH: Chip Select high */
	SPI_FLASH_CS_HIGH();
}

void W25Q16_PowerDown(SPI_HandleTypeDef* hspi) {
	uint8_t buffer[] = { W25X_PowerDown };
	/* Select the FLASH: Chip Select low */
	SPI_FLASH_CS_LOW();

	/* Send "Power Down" instruction */
	HAL_SPI_Transmit(hspi, buffer, sizeof(buffer), 100);

	/* Deselect the FLASH: Chip Select high */
	SPI_FLASH_CS_HIGH();
}

void W25Q16_WakeUp(SPI_HandleTypeDef* hspi) {
	uint8_t buffer[] = { W25X_ReleasePowerDown };
	/* Select the FLASH: Chip Select low */
	SPI_FLASH_CS_LOW();

	/* Send "Release Power Down" instruction */
	HAL_SPI_Transmit(hspi, buffer, sizeof(buffer), 100);

	/* Deselect the FLASH: Chip Select high */
	SPI_FLASH_CS_HIGH();
}

/*******************************************************************************
 * Function Name  : SPI_FLASH_SectorErase
 * Description    : Erases the specified FLASH sector.
 * Input          : SectorAddr: address of the sector to erase.
 * Output         : None
 * Return         : None
 *******************************************************************************/
void W25Q16_SectorErase(SPI_HandleTypeDef* hspi, uint32_t SectorAddr) {
	/* Send write enable instruction */
	SPI_FLASH_WriteEnable(hspi);
	SPI_FLASH_WaitForWriteEnd(hspi);
	/* Sector Erase */
	uint8_t buffer[] = {
			W25X_SectorErase,
			(SectorAddr & 0xFF0000) >> 16,
			(SectorAddr & 0xFF00) >> 8,
			SectorAddr & 0xFF
	};
	/* Select the FLASH: Chip Select low */
	SPI_FLASH_CS_LOW();
	/* Send Sector Erase instruction */
	HAL_SPI_Transmit(hspi, buffer, sizeof(buffer), 1000);
	/* Deselect the FLASH: Chip Select high */
	SPI_FLASH_CS_HIGH();
	/* Wait the end of Flash writing */
	SPI_FLASH_WaitForWriteEnd(hspi);
}

/*******************************************************************************
 * Function Name  : SPI_FLASH_BulkErase
 * Description    : Erases the entire FLASH.
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void W25Q16_ChipErase(SPI_HandleTypeDef* hspi) {
	uint8_t buffer[] = { W25X_ChipErase };
	/* Send write enable instruction */
	SPI_FLASH_WriteEnable(hspi);
	/* ChipErase */
	/* Select the FLASH: Chip Select low */
	SPI_FLASH_CS_LOW();
	/* Send Bulk Erase instruction  */
	HAL_SPI_Transmit(hspi, buffer, sizeof(buffer), 1000);
	/* Deselect the FLASH: Chip Select high */
	SPI_FLASH_CS_HIGH();

	/* Wait the end of Flash writing */
	SPI_FLASH_WaitForWriteEnd(hspi);
}

/*******************************************************************************
 * Function Name  : SPI_FLASH_PageWrite
 * Description    : Writes more than one byte to the FLASH with a single WRITE
 *                  cycle(Page WRITE sequence). The number of byte can't exceed
 *                  the FLASH page size.
 * Input          : - pBuffer : pointer to the buffer  containing the data to be
 *                    written to the FLASH.
 *                  - WriteAddr : FLASH's internal address to write to.
 *                  - NumByteToWrite : number of bytes to write to the FLASH,
 *                    must be equal or less than "SPI_FLASH_PageSize" value.
 * Output         : None
 * Return         : None
 *******************************************************************************/
static void SPI_FLASH_PageWrite(SPI_HandleTypeDef* hspi, uint8_t* pBuffer, uint32_t WriteAddr,
		uint16_t pSize) {
	uint8_t buffer[] = {
		W25X_PageProgram,
		(WriteAddr & 0xFF0000) >> 16,
		(WriteAddr & 0xFF00) >> 8,
		 WriteAddr & 0xFF
	};

	/* Enable the write access to the FLASH */
	SPI_FLASH_WriteEnable(hspi);

	/* Select the FLASH: Chip Select low */
	SPI_FLASH_CS_LOW();
	/* Send "Write to Memory " instruction */
	HAL_SPI_Transmit(hspi, buffer, sizeof(buffer), 1000);

//	if (pSize > SPI_FLASH_PerWritePageSize) {
//		pSize = SPI_FLASH_PerWritePageSize;
//		printf("\n\r Err: SPI_FLASH_PageWrite too large!");
//	}

	/* while there is data to be written on the FLASH */
	HAL_SPI_Transmit(hspi, pBuffer, pSize, 1000);

	/* Deselect the FLASH: Chip Select high */
	SPI_FLASH_CS_HIGH();

	/* Wait the end of Flash writing */
	SPI_FLASH_WaitForWriteEnd(hspi);
}

/*******************************************************************************
 * Function Name  : SPI_FLASH_BufferWrite
 * Description    : Writes block of data to the FLASH. In this function, the
 *                  number of WRITE cycles are reduced, using Page WRITE sequence.
 * Input          : - pBuffer : pointer to the buffer  containing the data to be
 *                    written to the FLASH.
 *                  - WriteAddr : FLASH's internal address to write to.
 *                  - NumByteToWrite : number of bytes to write to the FLASH.
 * Output         : None
 * Return         : None
 *******************************************************************************/
void W25Q16_BufferWrite(SPI_HandleTypeDef* hspi, uint8_t* pBuffer, uint32_t WriteAddr,
		uint16_t size) {
	uint8_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;

	Addr = WriteAddr % SPI_FLASH_PageSize;
	count = SPI_FLASH_PageSize - Addr;
	NumOfPage = size / SPI_FLASH_PageSize;
	NumOfSingle = size % SPI_FLASH_PageSize;

	if (Addr == 0) /* WriteAddr is SPI_FLASH_PageSize aligned  */
	{
		if (NumOfPage == 0) /* NumByteToWrite < SPI_FLASH_PageSize */
		{
			SPI_FLASH_PageWrite(hspi, pBuffer, WriteAddr, size);
		} else /* NumByteToWrite > SPI_FLASH_PageSize */
		{
			while (NumOfPage--) {
				SPI_FLASH_PageWrite(hspi, pBuffer, WriteAddr, SPI_FLASH_PageSize);
				WriteAddr += SPI_FLASH_PageSize;
				pBuffer += SPI_FLASH_PageSize;
			}

			SPI_FLASH_PageWrite(hspi, pBuffer, WriteAddr, NumOfSingle);
		}
	} else /* WriteAddr is not SPI_FLASH_PageSize aligned  */
	{
		if (NumOfPage == 0) /* NumByteToWrite < SPI_FLASH_PageSize */
		{
			if (NumOfSingle > count) /* (NumByteToWrite + WriteAddr) > SPI_FLASH_PageSize */
			{
				temp = NumOfSingle - count;

				SPI_FLASH_PageWrite(hspi, pBuffer, WriteAddr, count);
				WriteAddr += count;
				pBuffer += count;

				SPI_FLASH_PageWrite(hspi, pBuffer, WriteAddr, temp);
			} else {
				SPI_FLASH_PageWrite(hspi, pBuffer, WriteAddr, size);
			}
		} else /* NumByteToWrite > SPI_FLASH_PageSize */
		{
			size -= count;
			NumOfPage = size / SPI_FLASH_PageSize;
			NumOfSingle = size % SPI_FLASH_PageSize;

			SPI_FLASH_PageWrite(hspi, pBuffer, WriteAddr, count);
			WriteAddr += count;
			pBuffer += count;

			while (NumOfPage--) {
				SPI_FLASH_PageWrite(hspi, pBuffer, WriteAddr, SPI_FLASH_PageSize);
				WriteAddr += SPI_FLASH_PageSize;
				pBuffer += SPI_FLASH_PageSize;
			}

			if (NumOfSingle != 0) {
				SPI_FLASH_PageWrite(hspi, pBuffer, WriteAddr, NumOfSingle);
			}
		}
	}
}

/*******************************************************************************
 * Function Name  : SPI_FLASH_BufferRead
 * Description    : Reads a block of data from the FLASH.
 * Input          : - pBuffer : pointer to the buffer that receives the data read
 *                    from the FLASH.
 *                  - ReadAddr : FLASH's internal address to read from.
 *                  - NumByteToRead : number of bytes to read from the FLASH.
 * Output         : None
 * Return         : None
 *******************************************************************************/
void W25Q16_BufferRead(SPI_HandleTypeDef* hspi, uint8_t* pBuffer, uint32_t ReadAddr,
		uint16_t size) {

	uint8_t buffer[] = {
			W25X_ReadData,
			(ReadAddr & 0xFF0000) >> 16,
			(ReadAddr & 0xFF00) >> 8,
			ReadAddr & 0xFF
	};

	/* Select the FLASH: Chip Select low */
	SPI_FLASH_CS_LOW();

	/* Send "Read from Memory " instruction */
	HAL_SPI_Transmit(hspi, buffer, sizeof(buffer), 100);

	HAL_SPI_Receive(hspi, pBuffer, size, 1000);

	/* Deselect the FLASH: Chip Select high */
	SPI_FLASH_CS_HIGH();
}

/*******************************************************************************
 * Function Name  : SPI_FLASH_ReadID
 * Description    : Reads FLASH identification.
 * Input          : None
 * Output         : None
 * Return         : FLASH identification
 *******************************************************************************/
uint32_t W25Q16_ReadID(SPI_HandleTypeDef* hspi) {
	uint8_t buffer[] = { W25X_JedecDeviceID, Dummy_Byte, Dummy_Byte, Dummy_Byte,
	Dummy_Byte };

	/* Select the FLASH: Chip Select low */
	SPI_FLASH_CS_LOW();

	HAL_SPI_TransmitReceive(hspi, buffer, buffer, sizeof(buffer), 100);

	/* Deselect the FLASH: Chip Select high */
	SPI_FLASH_CS_HIGH();

	return (buffer[1] << 24) | (buffer[2] << 16) | (buffer[3] << 8) | buffer[4];
}
/*******************************************************************************
 * Function Name  : SPI_FLASH_ReadID
 * Description    : Reads FLASH identification.
 * Input          : None
 * Output         : None
 * Return         : FLASH identification
 *******************************************************************************/
uint32_t W25Q16_ReadDeviceID(SPI_HandleTypeDef* hspi) {
	uint8_t buffer[] = { W25X_DeviceID, Dummy_Byte, Dummy_Byte, Dummy_Byte,
	Dummy_Byte };

	/* Select the FLASH: Chip Select low */
	SPI_FLASH_CS_LOW();

	HAL_SPI_TransmitReceive(hspi, buffer, buffer, sizeof(buffer), 100);

	/* Deselect the FLASH: Chip Select high */
	SPI_FLASH_CS_HIGH();

	return buffer[4];
}

