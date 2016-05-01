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

void SetInterrupts()
{
	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
	HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 0, 0);
	HAL_NVIC_SetPriority(EXTI3_IRQn, 2, 0);
	HAL_NVIC_SetPriority(USART1_IRQn, 3, 0);
	HAL_NVIC_SetPriority(USART2_IRQn, 4, 0);
	HAL_NVIC_SetPriority(RTC_IRQn, 1, 0);
}

void PrintTime() 
{
	uint8_t hour, min, sec;
	char buf[10];
	RTC_GetTime(&hour, &min, &sec);
	sprintf(buf, "%02u:%02u:%02u", hour, min, sec);
	TM_ILI9341_Puts(10, 20, buf, &TM_Font_7x10, ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE);
}

inline static void LcdWrite(char *buf, size_t x, size_t y) {
	TM_ILI9341_Puts(x, y, buf, &TM_Font_7x10, ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE);
}

void CheckError(char * message, int ret)
{
	if (ret) {
		LcdWrite(message, 10, 10);
		while(1);
	}
}

int8_t UpdateTime() 
{
	int ret;
	uint8_t hour, minute, second;
	ret = esp8266_GetTime(&hour, &minute, &second);
	CheckError("GetTime failed!\0", ret);
	RTC_SetTime(hour + 2, minute, second);	
	return ret;
}



void GetIp()
{
	int8_t ret;
	char buf[BUF_MEM_SIZE];
	memset(buf, 0, BUF_MEM_SIZE);
	ret = esp8266_GetIp(buf);
	CheckError("GetIp Failed!\n", ret);
	LcdWrite(buf, 0, 40);	
}

int main(void)
{
	int ret;
	char buf[BUF_MEM_SIZE];
	SetInterrupts();
	set_leds();
	TM_ILI9341_Init();
	delay_init();
	xpt2046_init();
	RFID_Init();
	RTC_Init();
	ret = esp8266_Init();
	CheckError("esp8266 initalization failed!\0", ret);
	UpdateTime();
	PrintTime();
	GetIp();
	ret = esp8266_MakeAsServer();
	CheckError("esp8266_MakeAsServer failed!\0", ret);
	while(1) {
		ret = esp8266_ScanForData(buf);
		if (!ret) {
			TM_ILI9341_DrawRectangle(0, 100, 239, 319, ILI9341_COLOR_BLACK);
			LcdWrite(buf, 0, 100);
		}
		delay_ms(10000);
		PrintTime();		
	}
	return 0;
}
