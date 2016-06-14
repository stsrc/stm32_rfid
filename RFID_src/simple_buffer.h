#ifndef _SIMPLE_BUFFER_H_
#define _SIMPLE_BUFFER_H_
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define BUF_MEM_SIZE 512
#define TEMP_MEM_SIZE 32

/**
 * @addtogroup RFID_System libraries
 * @{
 */

/**
 * @defgroup simple_buffer
 * @brief simple circural buffer library.
 * 
 * Used by UART2 and esp8266 libraries.
 */




/**
 * @brief - simple_buffer struct, represents buffer.
 * @param lock, temp - if there was byte insert in time of searching, data could
 * be corrupted. To avoid this, temp buffer is used to store new incoming data,
 * and lock is set to 1 to signal tail variable is not actual (and to put data into temp).
 * @param ignore - sometimes there is not need for bytes. Ignore is a counter which
 * describes count of bytes to ignore.
 */
struct simple_buffer{
	volatile uint8_t lock;
	char temp[TEMP_MEM_SIZE];
	char memory[BUF_MEM_SIZE];
	size_t head;
	size_t tail;
	size_t ignore;
};

/**
 * @brief initalization of buffer.
 */
void buffer_init(struct simple_buffer *buf);

/**
 * @brief get byte from buffer. Returns byte by pointer.
 * @retval - 0 on success. -ENOMEM if there is no bytes to read.
 */
int8_t buffer_get_byte(struct simple_buffer* buf, uint8_t *byte);

/**
 * @brief set byte to buffer.
 * @retval - 0 on succes. -ENOMEM if there is no space.
 */
int8_t buffer_set_byte(struct simple_buffer* buf, uint8_t byte);	

/**
 * @brief set data to buffer.
 * @param text - data to set.
 * @param data_size - data_size.
 * @retval 0 if success, -ENOMEM if there is no space.
 */
int8_t buffer_set_text(struct simple_buffer *buf, const char *text, 
		       size_t data_size);

/**
 * @brief function for searching of some text. If function found label,
 * it copies to output buffer string from that label to limiter.
 * @param label - text to search and from which start coping.
 * @param limiter - text that ends coping.
 * @param output - buffer in which text is placed.
 */ 
int8_t buffer_SearchGetLabel(struct simple_buffer *buf, const char *label, 
			     const char *limiter, char *output);

/**
 * @brief function checks wether buffer is full.
 * @retval 0 if it is not full, 1 otherwise.
 */
int8_t buffer_IsFull(struct simple_buffer *buf);

/**
 * @brief function which empties buffer.
 */
void buffer_Reset(struct simple_buffer *buf);

/**
 * @brief sets how many bytes to add will be ignored.
 */
void buffer_SetIgnore(struct simple_buffer *buf, size_t ignore);
#endif
