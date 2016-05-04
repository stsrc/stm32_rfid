#ifndef _SIMPLE_BUFFER_H_
#define _SIMPLE_BUFFER_H_
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define BUF_MEM_SIZE 512
#define TEMP_MEM_SIZE 16
struct simple_buffer{
	uint8_t lock;
	char temp[TEMP_MEM_SIZE];
	char memory[BUF_MEM_SIZE];
	size_t head;
	size_t tail;
};

void buffer_init(struct simple_buffer *buf);
int8_t buffer_get_byte(struct simple_buffer* buf, uint8_t *byte);
int8_t buffer_set_byte(struct simple_buffer* buf, uint8_t byte);	
int8_t buffer_set_text(struct simple_buffer *buf, const char *text);
int8_t buffer_SearchGetLabel(struct simple_buffer *buf, const char *label, 
			     const char *limiter, char *output);
int8_t buffer_IsFull(struct simple_buffer *buf);
void buffer_Reset(struct simple_buffer *buf);
int8_t buffer_MoveTailToLabel(struct simple_buffer *buf, const char *label);
int8_t buffer_CopyToNearestSign(struct simple_buffer *buf, char* output, 
				const char sign);
int8_t buffer_CopyToNearestWord(struct simple_buffer *buf, char* output, 
				const char * word);
void buffer_CopyTillHead(struct simple_buffer *buf, char *output);
int8_t buffer_IsEmpty(struct simple_buffer *buf);
#endif
