/*
 * bsp_tests.c
 *
 *  Created on: Nov 10, 2017
 *      Author: anvol
 */

#include "bsp_tests.h"


Test_Result W25Q16_Test(SPI_HandleTypeDef* hspi){
	char *data = "TechMaker W25Q16 library test";
	char s[100] = {0};

	W25Q16_SectorErase(hspi, 0);
	W25Q16_BufferWrite(hspi, (uint8_t*)data, 0, strlen(data));

	W25Q16_BufferRead(hspi, (uint8_t*)s, 0, strlen(data));

	if (strcmp(data, s) == 0)
		return PASSED;
	else
		return FAILED;
}

