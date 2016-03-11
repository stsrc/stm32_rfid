#include <stdlib.h>
#include <string.h>
#include <stm32f1xx_hal_gpio.h>
#include <stm32f1xx_hal_rcc.h>
#include <stm32f1xx_hal_spi.h>
#include "tm_stm32f1_ili9341.h"
#include "xpt2046.h"
#include "FatFs/ff.h"
#include "FatFs/sd_diskio.h"
#include "SeeedRFID.h"
#include "delay.h"

int main(void)
{
	uint8_t CRC_test;
	char buf[50];
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
	
	TM_ILI9341_Init();
	xpt2046_init();
	delay_init();
	SeeedRFID_init();
	while(1) {
		delay_ms(1000);
		memset(buf, 0, sizeof(buf));
		TM_ILI9341_Fill(ILI9341_COLOR_BLACK);
		CRC_test = SeeedRFID_WaitAndGetData();
		TM_ILI9341_Puts(10, 10, SeeedRFID_CardNumber(), &TM_Font_7x10, 
				ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE);	
		if (CRC_test)
			TM_ILI9341_Puts(10, 30, "CRCs are equal!\0", &TM_Font_7x10, 
				ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE);
		else
			TM_ILI9341_Puts(10, 10, "CRCs are NOT equal!\0", &TM_Font_7x10, 
					ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE);	
	}
	return 0;
}
