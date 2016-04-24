#include "simple_buffer.h"


void buffer_init(struct simple_buffer *buf)
{
	memset(buf, 0, sizeof(struct simple_buffer));
}

static inline size_t buffer_IncrementCounter(size_t counter)
{
	return (counter + 1) % BUF_MEM_SIZE;
}

int8_t buffer_get_byte(struct simple_buffer* buf, uint8_t *byte)
{
	if (buf->tail == buf->head)
		return -ENOMEM;
	*byte = buf->memory[buf->tail];
	buf->tail = buffer_IncrementCounter(buf->tail);
	return 0;
}

int8_t buffer_set_byte(struct simple_buffer* buf, uint8_t byte)
{
	uint8_t temp = buffer_IncrementCounter(buf->head);
	if (buf->tail == temp)
		return -ENOMEM;
	buf->memory[buf->head] = byte;
	buf->head = temp;
	return 0;
}	

int8_t buffer_set_text(struct simple_buffer *buf, const char *text) 
{
	int8_t ret;
	size_t len = strlen(text);
	if (len >= BUF_MEM_SIZE)
		return -ENOMEM;
	if (len > BUF_MEM_SIZE - buf->head)
		return -ENOMEM;
	for (size_t i = 0; i < len; i++) {
		ret = buffer_set_byte(buf, text[i]);
		if (ret)
			return ret;
	}
	return 0;
}

int8_t buffer_SearchGetLabel(struct simple_buffer *buf, const char *command, 
			     char *output)
{	
	size_t cnt = 0;
	uint8_t byte = 0;
	uint8_t tail_old = buf->tail;
	while(buf->tail != buf->head) {
		buffer_get_byte(buf, &byte);
		if (command[cnt] == byte) {
			if (cnt == strlen(command) - 2)	{
				while(!buffer_get_byte(buf, &byte)) {
					*(output++) = byte;
					if ((*(output - 2) == 'O') && 
					    (*(output - 1) == 'K')) {
						*(output - 2) = '\0';
						*(output - 1) = '\0';
						return 0;
					}
				}
				buf->tail = tail_old;
				return -EBUSY;
			}
			cnt++;
		} else {
			cnt = 0;
		}
	}
	buf->tail = tail_old;
	return -EINVAL;
}
