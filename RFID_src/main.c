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
#include "esp8266.h"


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

void esp8266_test(const char *test, int ret) {
	if (ret) {
		TM_ILI9341_Puts(10, 70, (char *)test, 
				&TM_Font_7x10, ILI9341_COLOR_BLACK, 
				ILI9341_COLOR_WHITE);
		TM_ILI9341_Puts(10, 100, "NO OK!!! BUG!!!\0", 
				&TM_Font_7x10, ILI9341_COLOR_BLACK, 
				ILI9341_COLOR_WHITE);
		while(1);
	}
}

inline static void lcd_write(char *buf, size_t x, size_t y) {
	TM_ILI9341_Puts(x, y, buf, &TM_Font_7x10, ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE);
}

int8_t GetTime() 
{
	int ret;
	uint8_t hour, minute, second;
	ret = esp8266_GetTime(&hour, &minute, &second);
	if (ret) {
		lcd_write("GetTime failed!\0", 10, 10);
		while(1);
	}
	return ret;
}

int main(void)
{
	int ret;
	set_interrupts();
	set_leds();
	TM_ILI9341_Init();
	delay_init();
	xpt2046_init();
	RFID_Init();
	RTC_Init();
	ret = esp8266_Init();
	if (ret) {
		lcd_write("esp8266 initalization failed!\0", 10, 10);
		while(1);
	}
	GetTime();
	return 0;
}
