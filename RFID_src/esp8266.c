#include "esp8266.h"

#define ESP8266_RST_PORT	GPIOA
#define ESP8266_RST_PIN		GPIO_PIN_4

#define ESP8266_FW_PORT		GPIOA
#define ESP8266_FW_PIN		GPIO_PIN_8

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

void esp8266_Init() 
{
	esp8266_InitPins();
	esp8266_HardReset();
	UART_2_init();
	memset(&UART2_receive_buffer, 0, sizeof(struct simple_buffer));
	esp8266_Send("AT+GMR\r\n\0");
	delay_ms(500);
}

int8_t esp8266_Send(const char *data) 
{
	int8_t ret;
	ret = buffer_set_text(&UART2_transmit_buffer, data);
	if (ret)
		return ret;
	return 0;
}

int8_t esp8266_GetReply(const char *command, char *output)
{
	return buffer_SearchGetLabel(&UART2_receive_buffer, command, output);
}
