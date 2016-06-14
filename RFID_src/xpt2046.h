#ifndef _XPT2046_H_
#define _XPT2046_H_
#include <stm32f1xx.h>
#include <stm32f1xx_hal_rcc.h>
#include <stm32f1xx_hal_gpio.h>

#include "SPI.h"
#include "RTC.h"
#include "tm_stm32f1_ili9341.h"
#include "TIM2.h"

/**
 * @addtogroup RFID_System libraries
 * @{
 */

/**
 * @defgroup xpt2046
 * @brief xpt2046 library.
 * @{
 */
 

extern __IO uint8_t xpt2046_irq_flag;

/**
 * @brief Initalization function. It sets pins. SPI interface
 * must be enabled before calling this function (via SPI_1_init()). 
 */
void xpt2046_Init(void);

/**
 * @brief Function reads position from touchpanel
 * @param x - pointer to x coordinate of sensed touch.
 * @param y - pointer to y coordinate
 * @param z - pointer to z coordinate (pushing force).
 */
int xpt2046_read(uint16_t *x, uint16_t *y, uint16_t *z);

/**
 * @brief Function turns on interrupts. When screen is touched, 
 * interrupt routine is called.
 */
void xpt2046_InterruptOn(void);

#endif /* _XPT2046_H_ */
