#ifndef _UART_HPP_
#define _UART_HPP_

#include <stm32f1xx_hal_gpio.h>
#include <stm32f1xx_hal_rcc.h>
#include <stm32f1xx_hal_uart.h>
#include <stm32f1xx_hal_cortex.h>

#include "RTC.h"
#include "tm_stm32f1_ili9341.h"

HAL_StatusTypeDef UART_1_init();
HAL_StatusTypeDef UART_2_init();

HAL_StatusTypeDef UART_1_read(unsigned char *data, uint8_t len);
HAL_StatusTypeDef UART_2_transmit(uint8_t *data, uint8_t size);
HAL_StatusTypeDef UART_2_receive(uint8_t *data, uint8_t size);

extern __IO uint8_t UART_1_ready_flag;
extern __IO uint8_t UART_1_error_flag;
#endif
