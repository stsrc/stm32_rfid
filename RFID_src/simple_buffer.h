#ifndef _SIMPLE_BUFFER_H_
#define _SIMPLE_BUFFER_H_
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define BUF_MEM_SIZE 256

struct simple_buffer{
	char memory[BUF_MEM_SIZE];
	uint8_t head;
	uint8_t tail;
};

void buffer_init(struct simple_buffer *buf);
int8_t buffer_get_byte(struct simple_buffer* buf, uint8_t *byte);
int8_t buffer_set_byte(struct simple_buffer* buf, uint8_t byte);	
int8_t buffer_set_text(struct simple_buffer *buf, const char *text);

#endif
