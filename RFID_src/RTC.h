#ifndef _RTC_H_
#define _RTC_H_
#include <stm32f1xx.h>
#include <stm32f1xx_hal_rcc.h>
#include <stm32f1xx_hal_gpio.h>
#include <stm32f1xx_hal_rtc.h>
#include <stm32f1xx_hal_cortex.h>

#include "tm_stm32f1_ili9341.h"

extern __IO uint8_t RTC_second_flag;

HAL_StatusTypeDef RTC_Init();
void RTC_GetTime(uint8_t *hour, uint8_t *min, uint8_t *sec);
void RTC_SetTime(uint8_t hour, uint8_t min, uint8_t sec);
#endif
