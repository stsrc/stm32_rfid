#ifndef _USART_HPP_
#define _USART_HPP_

#include <stm32f1xx_hal_gpio.h>
#include <stm32f1xx_hal_rcc.h>
#include <stm32f1xx_hal_uart.h>

#ifndef bool
#define bool uint8_t
#define true 1
#define false 0
#endif

void USART_1_init();
void USART_1_listen();
bool USART_1_available();
unsigned char USART_1_read();

#endif
