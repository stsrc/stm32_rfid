#ifndef _DELAY_H_
#define _DELAY_H_
#include <stm32f1xx.h>
#include <core_cm3.h>
#include <stm32f1xx_hal.h>
void delay_init();
void delay_ms(uint32_t delay_in_ms);
#endif
