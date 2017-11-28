#ifndef XPT2046_H_
#define XPT2046_H_

#include "stm32f4xx_hal.h"

void XPT2046_update(SPI_HandleTypeDef *hspi, uint16_t *x, uint16_t *y);

#endif /* XPT2046_H_ */
