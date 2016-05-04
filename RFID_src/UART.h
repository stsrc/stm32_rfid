#ifndef _UART_HPP_
#define _UART_HPP_

#include <stm32f1xx_hal_gpio.h>
#include <stm32f1xx_hal_rcc.h>
#include <stm32f1xx_hal_uart.h>
#include <stm32f1xx_hal_cortex.h>

#include "RTC.h"
#include "tm_stm32f1_ili9341.h"
#include "RFID.h"
#include "simple_buffer.h"
#include "esp8266.h"

HAL_StatusTypeDef UART_1_init();
HAL_StatusTypeDef UART_2_init();

HAL_StatusTypeDef UART_1_read(uint8_t *data, uint8_t size);

void UART_1_set_irq(uint8_t set);
void UART_2_transmit();
extern __IO uint8_t UART_1_flag;

extern struct simple_buffer UART2_transmit_buffer;
extern struct simple_buffer UART2_receive_buffer;

#define ready_bit 1 << 0
#define error_bit 1 << 1

#endif
