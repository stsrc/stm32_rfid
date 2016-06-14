#ifndef _DELAY_H_
#define _DELAY_H_
#include <stm32f1xx.h>
#include <core_cm3.h>
#include <stm32f1xx_hal.h>

/**
 * @addtogroup RFID_System libraries
 * @{
 */

/**
 * @defgroup Delay
 * @brief Delay library
 * @{
 */

/**
 * @brief delay initalization
 */
void delay_init();
/**
 * @brief function to wait specific time in ms
 */
void delay_ms(uint32_t delay_in_ms);
#endif
