/**	
   ----------------------------------------------------------------------
    Copyright (c) 2016 Tilen Majerle

    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation
    files (the "Software"), to deal in the Software without restriction,
    including without limitation the rights to use, copy, modify, merge,
    publish, distribute, sublicense, and/or sell copies of the Software, 
    and to permit persons to whom the Software is furnished to do so, 
    subject to the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
    AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
    OTHER DEALINGS IN THE SOFTWARE.
   ----------------------------------------------------------------------
 */
#include "esp8266_ll.h"

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/*                                                                        */
/*    Edit file name to esp8266_ll.c and edit values for your platform    */
/*                                                                        */
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

uint8_t ESP8266_LL_USARTInit(uint32_t baudrate) {
	return (uint8_t)UART_2_init();
}

uint8_t ESP8266_LL_USARTSend(uint8_t* data, uint16_t count) {
	return (uint8_t)UART_2_transmit(data, count);
}

/* USART receive interrupt handler */
void USART2_IRQHandler (void) {
	uint8_t ch;
	/* If interrupt was trigered not because of data awaiting in receive register,
	 * interrupt routine returns now. */
	if(!(USART2->SR & USART_SR_RXNE))
		return;
	ch = USART2->DR;
	/* Send received character to ESP stack */
	ESP8266_DataReceived(&ch, 1);
}

void ESP8266_LL_PinsInit() {
	GPIO_InitTypeDef init;
	init.Mode = GPIO_MODE_OUTPUT_PP;
	init.Pull = GPIO_NOPULL;
	init.Speed = GPIO_SPEED_FREQ_HIGH;
	init.Pin = ESP8266_RST_PIN | ESP8266_FW_PIN;
	__HAL_RCC_GPIOA_CLK_ENABLE();
	HAL_GPIO_Init(ESP8266_RST_PORT, &init);
	HAL_GPIO_WritePin(ESP8266_RST_PORT, ESP8266_RST_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(ESP8266_FW_PORT, ESP8266_FW_PIN, GPIO_PIN_SET);
}
