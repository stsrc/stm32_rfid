#ifndef _TIM2_H_
#define _TIM2_H_

#include <stm32f1xx.h>
#include <stm32f1xx_hal_rcc.h>
#include <stm32f1xx_hal_cortex.h>
#include "tm_stm32f1_ili9341.h" 
#include "xpt2046.h"

/**
 * @addtogroup RFID_System libraries
 * @{
 */

/**
 * @defgroup TIM2
 * @brief TIM2 library.
 *
 * Timer 2 is used for time measurement, and tasks execution after
 * specific time interval. Functions of this modules are named easily,
 * so there is no need to comment them.
 */

/**
 * @brief Initalization function
 */
void TIM2_Init();

/**
 * @brief Clear LCD after time interval event set
 */
void TIM2_ClearLCDAfterTimeInterval(uint8_t sec);

/**
 * @brief Turn off LCD after time interval event set
 */
void TIM2_TurnOffLCDAfterTimeInterval(uint8_t sec);

/**
 * @brief Tun on RFID after time interval event set
 */
void TIM2_TurnOnRFIDAfterTimeInterval(uint8_t sec);

#endif
