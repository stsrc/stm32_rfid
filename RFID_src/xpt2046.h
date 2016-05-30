#ifndef _XPT2046_H_
#define _XPT2046_H_
#include <stm32f1xx.h>
#include <stm32f1xx_hal_rcc.h>
#include <stm32f1xx_hal_gpio.h>

#include "SPI.h"
#include "RTC.h"
#include "tm_stm32f1_ili9341.h"
#include "TIM2.h"

extern __IO uint8_t xpt2046_irq_flag;

void xpt2046_init(void);
int xpt2046_read(uint16_t *x, uint16_t *y, uint16_t *z);
void xpt2046_InterruptOn(void);

#endif /* _XPT2046_H_ */
