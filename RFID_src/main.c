#include <stdlib.h>
#include <string.h>
#include <stm32f1xx_hal_gpio.h>
#include <stm32f1xx_hal_rcc.h>
#include <stm32f1xx_hal_spi.h>
#include "tm_stm32f1_ili9341.h"
#include "xpt2046.h"
#include "FatFs/ff.h"
#include "FatFs/sd_diskio.h"
#include "RFID.h"
#include "RTC.h"
#include "delay.h"

void set_leds()
{
	GPIO_InitTypeDef gpio_str = 
	{
		GPIO_PIN_8 | GPIO_PIN_9,
		GPIO_MODE_OUTPUT_PP,
		GPIO_NOPULL,
		GPIO_SPEED_FREQ_MEDIUM		
	};
	
	__HAL_RCC_GPIOC_CLK_ENABLE();
	HAL_GPIO_Init(GPIOC, &gpio_str);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8 | GPIO_PIN_9, GPIO_PIN_RESET);
}

void set_interrupts()
{
	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
	HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 0, 0);
	HAL_NVIC_SetPriority(EXTI3_IRQn, 2, 0);
	HAL_NVIC_SetPriority(USART1_IRQn, 3, 0);
	HAL_NVIC_SetPriority(USART2_IRQn, 4, 0);
	HAL_NVIC_SetPriority(RTC_IRQn, 1, 0);
}

void print_time() 
{
	uint8_t hour, min, sec;
	char buf[10];
	RTC_get_time(&hour, &min, &sec);
	sprintf(buf, "%02u:%02u:%02u", hour, min, sec);
	TM_ILI9341_Puts(10, 20, buf, &TM_Font_7x10, ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE);
}

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

void esp8266_HardReset() {
	HAL_GPIO_WritePin(ESP8266_RST_PORT, ESP8266_RST_PIN, GPIO_PIN_RESET);
	delay_ms(25);
	HAL_GPIO_WritePin(ESP8266_RST_PORT, ESP8266_RST_PIN, GPIO_PIN_SET);
	delay_ms(500);	
}

void esp8266_Init() 
{
	esp8266_InitPins();
	UART_2_init();
	esp8266_HardReset();
}

void esp8266_test() 
{
	const char *data = "AT+GMR\r\n\0";
	int8_t ret;
	memset(&UART2_transmit_buffer, 0, sizeof(struct simple_buffer));
	memset(&UART2_receive_buffer, 0, sizeof(struct simple_buffer));
	ret = buffer_set_text(&UART2_transmit_buffer, data);
	UART_2_set_TXE_irq(1);
	delay_ms(500);
	TM_ILI9341_Puts(10, 50, UART2_receive_buffer.memory, &TM_Font_7x10, ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE);
}

int main(void)
{
	char buf[11];
	set_interrupts();
	set_leds();

	TM_ILI9341_Init();
	delay_init();
	xpt2046_init();
	RFID_Init();
	RTC_Init();
	esp8266_Init();
	esp8266_test();
	RFID_Read();

	while(1) {
		if(READ_BIT(UART_1_flag, ready_bit)) {
			CLEAR_BIT(UART_1_flag, ready_bit);
			RFID_CardNumber(buf);
			TM_ILI9341_Puts(10, 10, buf, &TM_Font_7x10, 
					ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE);
		} else if (READ_BIT(UART_1_flag, error_bit)) {
			CLEAR_BIT(UART_1_flag, error_bit);
		} else if (RTC_second_flag) {
			RTC_second_flag = 0;
			print_time();
		}
	}
	return 0;
}
