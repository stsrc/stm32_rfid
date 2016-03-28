#include "UART.h"
#include "RFID.h"

static UART_HandleTypeDef uart_1_handler, uart_2_handler;
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

void USART1_IRQHandler(void) 
{
	HAL_UART_IRQHandler(&uart_1_handler);
}

HAL_StatusTypeDef UART_1_init() {
	UART_InitTypeDef init;
	init.BaudRate = 9600;
	init.WordLength = UART_WORDLENGTH_8B;
	init.StopBits = UART_STOPBITS_1;
	init.Parity = UART_PARITY_NONE;
	init.Mode = UART_MODE_RX;
	init.HwFlowCtl = UART_HWCONTROL_NONE;
	init.OverSampling = UART_OVERSAMPLING_16;

	uart_1_handler.Init = init;
	uart_1_handler.Instance = USART1;
		
	return HAL_UART_Init(&uart_1_handler);
}

void HAL_UART_MspInit(UART_HandleTypeDef *huart) {
	GPIO_InitTypeDef init_gpio;
	if (huart->Instance == USART1) init_gpio.Pin = GPIO_PIN_9;
	else if (huart->Instance == USART2) init_gpio.Pin = GPIO_PIN_2;
	init_gpio.Pull = GPIO_NOPULL;
	init_gpio.Mode = GPIO_MODE_AF_PP;
	init_gpio.Speed = GPIO_SPEED_FREQ_HIGH;
	__HAL_RCC_GPIOA_CLK_ENABLE();
	HAL_GPIO_Init(GPIOA, &init_gpio);	
	if (huart->Instance == USART1) init_gpio.Pin = GPIO_PIN_10;
	else if (huart->Instance == USART2) init_gpio.Pin = GPIO_PIN_3;
	init_gpio.Pull = GPIO_PULLDOWN;
	init_gpio.Mode = GPIO_MODE_AF_INPUT;
	HAL_GPIO_Init(GPIOA, &init_gpio);
	
	if (huart->Instance == USART1) { 
		__HAL_RCC_USART1_CLK_ENABLE();
		HAL_NVIC_EnableIRQ(USART1_IRQn);
	} else if (huart->Instance == USART2) {
		__HAL_RCC_USART2_CLK_ENABLE();
		HAL_NVIC_EnableIRQ(USART2_IRQn);
	}
}

HAL_StatusTypeDef UART_1_read(unsigned char* data, uint8_t len) {
	return HAL_UART_Receive_IT(&uart_1_handler, data, len);
}

HAL_StatusTypeDef UART_2_init() {
	HAL_StatusTypeDef ret;
	UART_InitTypeDef init;
	init.BaudRate = 115200;
	init.WordLength = UART_WORDLENGTH_8B;
	init.StopBits = UART_STOPBITS_1;
	init.Parity = UART_PARITY_NONE;
	init.Mode = UART_MODE_TX_RX;
	init.HwFlowCtl = UART_HWCONTROL_NONE; //maybe use hw control?
	init.OverSampling = UART_OVERSAMPLING_16;

	uart_2_handler.Init = init;
	uart_2_handler.Instance = USART2;	
	ret = HAL_UART_Init(&uart_2_handler);
	if (ret)
		return ret;
	/* Interrupt enable */
	USART2->CR1 |= USART_CR1_PEIE;
	/* Interrupt on data reception */
	USART2->CR1 |= USART_CR1_RXNEIE;	
	return HAL_OK;
}

HAL_StatusTypeDef UART_2_transmit(uint8_t* data, uint8_t size) {
	return HAL_UART_Transmit(&uart_2_handler, data, size, 0xFFFF);
}

HAL_StatusTypeDef UART_2_receive(uint8_t* data, uint8_t size) {
	return HAL_UART_Receive(&uart_2_handler, data, size, 0xFFFF);
}


