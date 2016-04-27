#include "esp8266.h"

#define ESP8266_RST_PORT	GPIOA
#define ESP8266_RST_PIN		GPIO_PIN_4

#define ESP8266_FW_PORT		GPIOA
#define ESP8266_FW_PIN		GPIO_PIN_8

#define AT_RESET_CMD		"AT+RST\r\n\0"

void esp8266_InitPins() 
{
	GPIO_InitTypeDef init;
	init.Mode = GPIO_MODE_OUTPUT_PP;
	init.Pull = GPIO_NOPULL;
	init.Speed = GPIO_SPEED_FREQ_HIGH;
	init.Pin = ESP8266_RST_PIN | ESP8266_FW_PIN;
	__HAL_RCC_GPIOA_CLK_ENABLE();
	HAL_GPIO_Init(ESP8266_RST_PORT, &init);
	HAL_GPIO_WritePin(ESP8266_FW_PORT, ESP8266_FW_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(ESP8266_RST_PORT, ESP8266_RST_PIN, GPIO_PIN_SET);
}

void esp8266_HardReset() 
{
	HAL_GPIO_WritePin(ESP8266_RST_PORT, ESP8266_RST_PIN, GPIO_PIN_RESET);
	delay_ms(50);
	HAL_GPIO_WritePin(ESP8266_RST_PORT, ESP8266_RST_PIN, GPIO_PIN_SET);
	delay_ms(500);	
}

int8_t esp8266_WaitForOk(const char *command, unsigned int delay, uint8_t multiplier) 
{
	int8_t ret;
	char temp[BUF_MEM_SIZE];
	ret = esp8266_GetReply(command, temp, delay, multiplier);
	return ret;	
}

void esp8266_Init() 
{
	const char * const RST_CMD = AT_RESET_CMD;
	esp8266_InitPins();
	esp8266_HardReset();
	UART_2_init();
	buffer_Reset(&UART2_transmit_buffer);
	delay_ms(3000);
	buffer_Reset(&UART2_receive_buffer);
	esp8266_Send(RST_CMD);
	esp8266_WaitForOk(RST_CMD, 100, 10);
	buffer_Reset(&UART2_receive_buffer);
}

int8_t esp8266_Send(const char *data) 
{
	int8_t ret;
	ret = buffer_set_text(&UART2_transmit_buffer, data);
	if (!ret)
		UART_2_transmit();
	return ret;
}

int8_t esp8266_SendGetReply(const char *command, char *output)
{
	int8_t ret;
	if (buffer_IsFull(&UART2_transmit_buffer))
		return -ENOMEM;
	buffer_Reset(&UART2_receive_buffer);		
	esp8266_Send(command);
	do {
		ret = buffer_SearchGetLabel(&UART2_receive_buffer, command, output);
	} while (ret == -EBUSY);
	return ret;
}

int8_t esp8266_GetReply(const char *command, char *output, unsigned int delay, uint8_t multiplier)
{
	int8_t ret, cnt = 0;
	do {
		delay_ms(delay);
		ret = buffer_SearchGetLabel(&UART2_receive_buffer, command, output);
	} while (((ret == -EBUSY) || (ret == -EINVAL)) && (++cnt < multiplier));
	return ret;
}
