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
	if (buf->head >= buf->tail) {
		if (len >= BUF_MEM_SIZE - (buf->head - buf->tail))
			return -ENOMEM;
	} else if (buf-> head < buf->tail) {
		if (len >= BUF_MEM_SIZE - (buf->tail - buf->head))
			return -ENOMEM;
	}
	for (size_t i = 0; i < len; i++) {
		ret = buffer_set_byte(buf, text[i]);
		if (ret)
			return ret;
	}
	return 0;
}

int8_t buffer_IsFull(struct simple_buffer *buf) 
{
	uint8_t temp = buffer_IncrementCounter(buf->head);
	if (buf->tail == temp)
		return -ENOMEM;
	else
		return 0;
}

int8_t buffer_MoveTailToLabel(struct simple_buffer *buf, const char *label)
{
	uint8_t byte;
	int8_t ret;
	size_t cnt = 0;
	size_t tail_old = buf->tail;
	while (buf->tail != buf->head) {
		ret = buffer_get_byte(buf, &byte);
		if (ret == -ENOMEM) {
			buf->tail = tail_old;
			return -EBUSY;
		}
		if (label[cnt] == byte) {
			if (cnt == strlen(label) - 3)
				return 0;
			else 
				cnt++;
		} else {
			cnt = 0;
		}
	}	
	buf->tail = tail_old;
	return -EINVAL;
}

int8_t buffer_SearchGetLabel(struct simple_buffer *buf, const char *command, 
			     char *output)
{	
	int8_t ret;
	size_t tail_old = buf->tail;
	ret = buffer_MoveTailToLabel(buf, command);
	if (ret)
		return ret;
	ret = buffer_CopyToNearestWord(buf, output, "OK\0");
	if (ret)
		buf->tail = tail_old;
	return ret;
}

int8_t buffer_CopyToNearestSign(struct simple_buffer *buf, char* output, 
				const char sign)
{
	uint8_t byte = 0;
	size_t tail_old = buf->tail;
	while(!buffer_get_byte(buf, &byte)) {
		if (byte == sign) {
			*output = '\0';
			return 0;
		}
		*output++ = byte;
	}
	buf->tail = tail_old;
	return -EINVAL;
}

int8_t buffer_CopyToNearestWord(struct simple_buffer *buf, char *output,
				const char *word)
{
	uint8_t byte = 0;
	size_t len = strlen(word);
	size_t cnt = 0;
	int8_t ret;
	size_t tail_old = buf->tail;
	while(!(ret = buffer_get_byte(buf, &byte))) {
		*output = byte;
		if (*output == word[len - 1]) {
			for (cnt = 0; cnt < len; cnt++) {
				if (*(output - cnt) != word[len - 1 - cnt])
					break;
			}
			if (cnt == len) {
				for (int i = len - 1; i >= 0; i--)
					*output-- = '\0';
				return 0;
			}
		}
		output++;
	}
	buf->tail = tail_old;
	if (ret == -ENOMEM)
		ret = -EBUSY;
	return ret;
}

void buffer_Reset(struct simple_buffer *buf) 
{
	memset(buf, 0, sizeof(struct simple_buffer));
}

void buffer_CopyTillHead(struct simple_buffer *buf, char *output) 
{
	uint8_t byte;
	while(!buffer_get_byte(buf, &byte))
		*output++ = byte;
}
