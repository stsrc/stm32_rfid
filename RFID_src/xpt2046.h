#ifndef _XPT2046_H_
#define _XPT2046_H_
#include <stm32f1xx.h>
#include <stm32f1xx_hal_rcc.h>
#include <stm32f1xx_hal_gpio.h>

#include "SPI.h"
#include "RTC.h"
#include "tm_stm32f1_ili9341.h"

void xpt2046_init(void);
int xpt2046_read(uint16_t *x, uint16_t *y, uint16_t *z);

#endif /* _XPT2046_H_ */
