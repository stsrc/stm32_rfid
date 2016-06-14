#ifndef _SPI_H_
#define _SPI_H_

#include <stdlib.h>
#include <string.h>
#include <stm32f1xx_hal_gpio.h>
#include <stm32f1xx_hal_rcc.h>
#include <stm32f1xx_hal_spi.h>
#include <stm32f1xx_hal_dma.h>
#include <stm32f1xx_hal_cortex.h>

void SPI_show_error(HAL_StatusTypeDef rt);

HAL_StatusTypeDef SPI_1_init();
HAL_StatusTypeDef SPI_1_send(uint8_t *data);
HAL_StatusTypeDef SPI_1_read(uint8_t *data, uint16_t bytes);
HAL_StatusTypeDef SPI_1_DMA_send(uint8_t *data, uint16_t bytes);
/** @brief function changes speed of SPI_1.
 * @param old - pointer to variable in which old speed whill be stored. 
 * May be NULL.
 * @param new - new speed.
 */
HAL_StatusTypeDef SPI_1_change_speed(uint32_t *old, uint32_t new);

HAL_StatusTypeDef SPI_2_init();
HAL_StatusTypeDef SPI_2_write(uint8_t *data);
HAL_StatusTypeDef SPI_2_read(uint8_t *data, uint16_t bytes);


#endif

/* TODO unification ! */
