#ifndef _UART_HPP_
#define _UART_HPP_

#include <stm32f1xx_hal_gpio.h>
#include <stm32f1xx_hal_rcc.h>
#include <stm32f1xx_hal_uart.h>
#include <stm32f1xx_hal_cortex.h>

#include "RTC.h"
#include "tm_stm32f1_ili9341.h"
#include "RFID.h"

HAL_StatusTypeDef UART_1_init();
HAL_StatusTypeDef UART_2_init();

HAL_StatusTypeDef UART_1_read(uint8_t *data, uint8_t size);
HAL_StatusTypeDef UART_2_transmit(uint8_t *data, uint16_t size);
HAL_StatusTypeDef UART_2_receive(uint8_t *data, uint16_t size);

void UART_1_set_irq(uint8_t set);

extern __IO uint8_t UART_1_flag;
#define ready_bit 1 << 0
#define error_bit 1 << 1

#endif
