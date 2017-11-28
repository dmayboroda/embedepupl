#include "XPT2046.h"

void XPT2046_update(SPI_HandleTypeDef *hspi, uint16_t *x, uint16_t *y) {
	uint8_t buffer[] = { 0x90, 0x00, 0xD0, 0x00, 0x00 };

	if (HAL_GPIO_ReadPin(T_PEN_GPIO_Port, T_PEN_Pin))
		return;

	HAL_SPI_TransmitReceive(hspi, buffer, buffer, sizeof(buffer), 1000);

	*x = (buffer[1] << 5) + (buffer[2] >> 3); // make uint16 from two bytes MSB first
	*y = (buffer[3] << 5) + (buffer[4] >> 3);
}
