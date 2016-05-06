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
	HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 13, 0);
	HAL_NVIC_SetPriority(EXTI3_IRQn, 15, 0);
	HAL_NVIC_SetPriority(USART1_IRQn, 1, 0);
	HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
	HAL_NVIC_SetPriority(RTC_IRQn, 14, 0);
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

void GetIp(char *buf)
{
	int8_t ret;
	memset(buf, 0, BUF_MEM_SIZE);
	ret = esp8266_GetIp(buf);
	CheckError("GetIp Failed!\n", ret);
	LcdWrite(buf, 0, 40);	
}

int8_t FATFS_Init(FATFS *SDFatFs, char *path)
{
	int8_t ret;
	ret = FATFS_LinkDriver((Diskio_drvTypeDef *)&SD_Driver, path);
	if (ret)
		return -1;
	ret = f_mount(SDFatFs, path, 1);
	if (ret)
		return -2;
	return ret;
}

int8_t WiFi_Init()
{
	int ret = 0;
	const size_t max = 3;
	for (size_t i = 0; i < max; i++) {
		ret = esp8266_Init();
		if (!ret) {
			break;
		} else if (i != max - 1) {
			LcdWrite("Problems with esp8266 \ninitalization.\nRetrying in 3 sec.", 0, 0);
			delay_ms(3000);
			TM_ILI9341_DrawFilledRectangle(0, 0, 239, 50, ILI9341_COLOR_BLACK);	
		}
	}
	return ret;
}

int8_t WritePage(char *buf)
{
	uint8_t id;
	int8_t ret;
	FIL html_file;
	size_t bytes_read;
	volatile size_t file_size = 0;

	ret = esp8266_ScanForFile(buf, &id);
	if (ret)
		return 0;
	
	if (!strlen(buf))
		strcpy(buf, "index.html");
	
	f_open(&html_file, buf, FA_OPEN_EXISTING | FA_READ);
	if (ret)
		return -1;
	
	file_size = f_size(&html_file);

	while(file_size > BUF_MEM_SIZE - 1) {
	
		memset(buf, 0, BUF_MEM_SIZE);
	
		ret = f_read(&html_file, buf, BUF_MEM_SIZE - 1, 
			     (UINT *)&bytes_read);
		if (ret) {
			f_close(&html_file);
			return -2;
		}
	
		file_size -= bytes_read;
		ret = esp8266_WritePage(buf, bytes_read, id, 0);
		if (ret) {
			f_close(&html_file);
			return -3;
		}

	}

	memset(buf, 0, BUF_MEM_SIZE);
	
	ret = f_read(&html_file, buf, file_size, (UINT *)&bytes_read);
	if (ret) {
		f_close(&html_file);
		return -4;
	}

	ret = esp8266_WritePage(buf, file_size, id, 1);
	if (ret) {
		f_close(&html_file);
		return -5;
	}

	ret = f_close(&html_file);
	if (ret)
		return -6;

	return 0;
}

FATFS SDFatFs;
int main(void)
{
	int ret;
	char buf[BUF_MEM_SIZE];
	char org[5];

	memset(org, 0, 5);
	SetInterrupts();
	set_leds();
	TM_ILI9341_Init();
	delay_init();
	xpt2046_init();
	RFID_Init();
	RTC_Init();
	ret = FATFS_Init(&SDFatFs, org);
	CheckError("FATFS initalization failed!\0", ret);
	ret = WiFi_Init();
	CheckError("Can not connect to WiFi!\0", ret);
	UpdateTime();
	PrintTime();
	GetIp(buf);
	ret = esp8266_MakeAsServer();
	CheckError("esp8266_MakeAsServer failed!\0", ret);
	while(1) {
		if(esp8266_CheckResetFlag()) {
			ret = WiFi_Init();
			CheckError("Can not reset WiFi!", ret);
			ret = esp8266_MakeAsServer();
			CheckError("esp8266_MakeAsServer failed!\0", ret);
		}
		WritePage(buf);
	}
	return 0;
}
