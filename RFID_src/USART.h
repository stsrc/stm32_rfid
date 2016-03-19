#ifndef _USART_HPP_
#define _USART_HPP_

#include <stm32f1xx_hal_gpio.h>
#include <stm32f1xx_hal_rcc.h>
#include <stm32f1xx_hal_uart.h>
#include <stm32f1xx_hal_cortex.h>

#include "RTC.h"
#include "tm_stm32f1_ili9341.h"

void USART_1_init();
void USART_1_listen();
uint8_t USART_1_available();
void USART_1_read(unsigned char *data, uint8_t len);

extern __IO uint8_t USART_1_ready;
#endif
