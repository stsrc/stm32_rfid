#include <stdlib.h>
#include <string.h>
#include <stm32f1xx_hal_gpio.h>
#include <stm32f1xx_hal_rcc.h>
#include <stm32f1xx_hal_spi.h>
#include "tm_stm32f1_ili9341.h"
#include "SPI.h"
#include "xpt2046.h"
#include "FatFs/ff.h"
#include "FatFs/sd_diskio.h"

int main(void)
{
	uint8_t ret;
	uint32_t bytes_written;
	char *text_to_write = "Testing Fatfs\0";
	FATFS SDFatFs;
	FIL MyFile;
	uint16_t x, y, z;
	char sd_path[100];
	char buf[50];
	GPIO_InitTypeDef gpio_str = 
	{
		GPIO_PIN_8 | GPIO_PIN_9,
		GPIO_MODE_OUTPUT_PP,
		GPIO_NOPULL,
		GPIO_SPEED_FREQ_MEDIUM		
	};
	
	memset(sd_path, 0, sizeof(sd_path));

	__HAL_RCC_GPIOC_CLK_ENABLE();
	HAL_GPIO_Init(GPIOC, &gpio_str);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8 | GPIO_PIN_9, GPIO_PIN_RESET);
	
	TM_ILI9341_Init();
	xpt2046_init();

	ret = FATFS_LinkDriver((Diskio_drvTypeDef *)&SD_Driver, sd_path);
	if (ret != FR_OK)
		goto err;
	ret = f_mount(&SDFatFs, sd_path, 1);
	if (ret != FR_OK) 
		goto err;
	ret = f_open(&MyFile, "test.txt", FA_CREATE_ALWAYS | FA_WRITE);
  if (ret != FR_OK)
		goto err;	
	ret = f_write(&MyFile, text_to_write, strlen(text_to_write) + 1, (UINT *)&bytes_written);
	if (ret != FR_OK)
		goto err;
	ret = f_close(&MyFile);
	if (ret != FR_OK)
		goto err;
	ret = FATFS_UnLinkDriver(sd_path);
	if (ret != FR_OK)
		goto err;
	
	while(1) {
		for(int it = 0; it < 1000; it++);
		xpt2046_read(&x, &y, &z);
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "x = %03hu, y = %03hu, z = %03hu", x, y, z);
		TM_ILI9341_Puts(10, 10, buf, &TM_Font_7x10, 
				ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE);
			
	}
	return 0;
err:
	SPI_show_error(HAL_ERROR);
	return 1;
}
