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

int main(void)
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
	
	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

	TM_ILI9341_Init();
	delay_init();
	xpt2046_init();
	RFID_Init();
	RTC_Init();

	RFID_Read();
	
	while(1) {
		if(UART_1_ready) {
			UART_1_ready = 0;
			TM_ILI9341_Puts(10, 10, RFID_CardNumber(), &TM_Font_7x10, 
					ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE);	
			RFID_Read();
		}	
	}
	return 0;
}
