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
	char test[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 0};
	TM_ILI9341_Puts(10, 11, test, &TM_Font_7x10, ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE);
	while(1) {
		delay_ms(250);
		memset(buf, 0, sizeof(buf));
		SeeedRFID_WaitAndGetData();
		TM_ILI9341_Puts(10, 10, SeeedRFID_CardNumber(), &TM_Font_7x10, 
				ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE);	
	}
	return 0;
}
