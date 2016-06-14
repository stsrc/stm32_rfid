#ifndef _ESP8266_H_
#define _ESP8266_H_

#include <inttypes.h>
#include <stm32f1xx_hal_gpio.h>
#include "UART.h"
#include "delay.h"
#include "secret_password.h"

/**
 * @addtogroup RFID_System libraries
 * @{
 */

/**
 * @defgroup esp8266
 * @brief esp8266 library
 * @{
 */


/**
 * @brief Initialization of esp8266
 *
 * Function sets esp8266 pins, turns on UART2, 
 * changes speed of esp8266 and UART2, to achive higher transfer,
 * and connects to Wi-Fi. Password to Wi-Fi is in secret_password.h file.
 */
int8_t esp8266_Init(); 

/**
 * @brief Function to get date from Internet by WWW.
 *
 * Function sends page request to google.com. It responds with time and date, 
 * which is then parsed to variables in pointers.
 *
 * @param day
 * @param month
 * @param year
 * @param ... - pointers to variable which function will update with actual 
 *		values
 */
int8_t esp8266_GetDate(uint8_t *day, uint8_t *month, uint16_t *year, 
		       uint8_t *hour, uint8_t *minute, uint8_t *second);

/**
 * @param buf - buffer to which function will write IP and MAC.
 * @retval 0 on success, negative value on fail.
 */
int8_t esp8266_GetIp(char *buf);

/**
 * @brief creates TCP/IP server from esp8266.
 * @retval please look to source code.
 */
int8_t esp8266_MakeAsServer();

/**
 * @brief HTTP requests check
 * @param file - buffer in which name of file to be send will be placed.
 * @param id - id which is linked with file to be send.
 * @retval 0 if there is pending HTTP request, -EINVAL otherwise.
 */
int8_t esp8266_ScanForFile(char *file, uint8_t *id);

/**
 * @brief Reset condition check.
 * @retval true if esp8266 must be reset, false otherwise.
 */ 
int8_t esp8266_CheckResetFlag();

/**
 * @brief AT+CIPSEND command write for data transmit.
 *
 * Function firstly sends AT+CIPSEND command, waits for acknowledge, then it
 * transmits data, and again waits for acknowledge from esp8266 chip.
 *
 * @param data - payload of chunk to be written.
 * @data_size - data size
 * @id - channel to which data will be send.
 * @retval -1 if channel is not in transmit state (serious error),
 * -2 if send of command failed, -3 if wait for command result failed,
 *  -4 is send of data failed, 0 on success.
 */
int8_t esp8266_WriteATCIPSEND(char *data, size_t data_size, uint8_t id);

/**
 * @brief AT+CIPCLOSE command write. 
 *
 * Used to close a channel.
 * 
 * @param buf - temporary buffer, of size which equals to BUF_MEM_SIZE
 * @param id - id of channel to be closed.
 * @retval -1 if send of AT+CIPCLOSE failed, -2 if waiting for command result 
 * failed, 0 on success.
 */
int8_t esp8266_WriteATCIPCLOSE(char *buf, uint8_t id);

/**
 * @brief Function which waits for acknowledge to ATCIPSEND
 * or ATCIPCLOSE function.
 *
 * @param command - command, that is waiting for.
 * @delay - delay in ms between searches of command in input buffer.
 * @multiplier - count of function repetition. First function searches
 * command, if there is no such command it waits for delay. Then multiplier
 * defines how many such a loops to make.
 *
 * @retval  0 if command ended with OK, 1 if command ended with FAIL, 2
 * if command ended wich ERROR, -EINVAL if there was no such command,
 * -EBUSY if there is i.e. half of command and nothing more.
 */
int8_t esp8266_WaitForAck(const uint8_t id, const char *command, 
			  unsigned int delay, uint8_t multiplier); 

/**
 * @brief Function to clear reset flag after Wi-Fi reset.
 */
void esp8266_ClearResetFlag();
#endif
