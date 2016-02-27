#ifndef _SPI_H_
#define _SPI_H_

#include <stdlib.h>
#include <string.h>
#include <stm32f1xx_hal_gpio.h>
#include <stm32f1xx_hal_rcc.h>
#include <stm32f1xx_hal_spi.h>


void SPI_check_return(HAL_StatusTypeDef rt);
void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi);
HAL_StatusTypeDef SPI_1_init();
HAL_StatusTypeDef SPI_1_send(uint8_t *data);



#endif
