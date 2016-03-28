#include "UART.h"
#include "RFID.h"

static UART_HandleTypeDef uart_handler;
__IO uint8_t UART_1_ready_flag = 0;
__IO uint8_t UART_1_error_flag = 0;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART1)
		UART_1_ready_flag = 1;
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) 
{
	if (huart->Instance == USART1)
		UART_1_error_flag = 1;	
}

void USART1_IRQHandler(void) {
	HAL_UART_IRQHandler(&uart_handler);
}

void UART_1_init() {
	UART_InitTypeDef init;
	init.BaudRate = 9600;
	init.WordLength = UART_WORDLENGTH_8B;
	init.StopBits = UART_STOPBITS_1;
	init.Parity = UART_PARITY_NONE;
	init.Mode = UART_MODE_RX;
	init.HwFlowCtl = UART_HWCONTROL_NONE;
	init.OverSampling = UART_OVERSAMPLING_16;

	uart_handler.Init = init;
	uart_handler.Instance = USART1;
		
	HAL_UART_Init(&uart_handler);
}

void HAL_UART_MspInit(UART_HandleTypeDef *huart) {
	GPIO_InitTypeDef init_gpio;
	init_gpio.Pin = GPIO_PIN_9;
	init_gpio.Pull = GPIO_NOPULL;
	init_gpio.Mode = GPIO_MODE_AF_PP;
	init_gpio.Speed = GPIO_SPEED_FREQ_HIGH;
	__HAL_RCC_GPIOA_CLK_ENABLE();
	HAL_GPIO_Init(GPIOA, &init_gpio);	
	init_gpio.Pin = GPIO_PIN_10;
	init_gpio.Pull = GPIO_PULLDOWN;
	init_gpio.Mode = GPIO_MODE_AF_INPUT;
	HAL_GPIO_Init(GPIOA, &init_gpio);
	
	__HAL_RCC_USART1_CLK_ENABLE();
	HAL_NVIC_EnableIRQ(USART1_IRQn);
}

void UART_1_listen() {
 while(!(USART1->SR & USART_SR_RXNE));
}

uint8_t UART_1_available() {
	return USART1->SR & USART_SR_RXNE;
}

HAL_StatusTypeDef UART_1_read(unsigned char* data, uint8_t len) {
	return HAL_UART_Receive_IT(&uart_handler, data, len);
}


