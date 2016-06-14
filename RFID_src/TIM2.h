#ifndef _TIM2_H_
#define _TIM2_H_

#include <stm32f1xx.h>
#include <stm32f1xx_hal_rcc.h>
#include <stm32f1xx_hal_cortex.h>
#include "tm_stm32f1_ili9341.h" 
#include "xpt2046.h"

/**
 * @brief timer for some time based events
 */

void TIM2_Init();
void TIM2_ClearLCDAfterTimeInterval(uint8_t sec);
void TIM2_TurnOffLCDAfterTimeInterval(uint8_t sec);
void TIM2_TurnOnRFIDAfterTimeInterval(uint8_t sec);

#endif
