#include "simple_buffer.h"

void buffer_init(struct simple_buffer *buf)
{
	memset(buf, 0, sizeof(struct simple_buffer));
}

int8_t buffer_get_byte(struct simple_buffer* buf, uint8_t *byte)
{
	if (buf->tail == buf->head)
		return -ENOMEM;
	*byte = buf->memory[buf->tail];
	buf->tail = (buf->tail + 1) % BUF_MEM_SIZE;
	return 0;
}

int8_t buffer_set_byte(struct simple_buffer* buf, uint8_t byte)
{
	uint8_t temp = (buf->head + 1) % BUF_MEM_SIZE;
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
