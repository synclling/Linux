#ifndef __CIRCULAR_BUFFER_H__
#define __CIRCULAR_BUFFER_H__

#include <stdint.h>

typedef struct circular_buffer
{
	int size;
	int capacity;
	int read;
	int write;
	uint8_t *data;
} circular_buffer_t;

circular_buffer_t *circular_buffer_init(int capacity);

void circular_buffer_uninit(circular_buffer_t *circular_buf);

void circular_buffer_reset(circular_buffer_t *circular_buf);

int circular_buffer_read(circular_buffer_t *circular_buf, uint8_t *buf, int size);

int circular_buffer_write(circular_buffer_t *circular_buf, const uint8_t *buf, int size);

int circular_buffer_size(circular_buffer_t *circular_buf);

int circular_buffer_capacity(circular_buffer_t *circular_buf);

int circular_buffer_get_by_index(circular_buffer_t *circular_buf, int index, uint8_t *value);

void circular_buffer_pop(circular_buffer_t *circular_buf, const int n);

#endif
