#include "circular_buffer.h"
#include <string.h>
#include <stdlib.h>

#define MIN(a, b)	(a) < (b)? (a) : (b)

circular_buffer_t *circular_buffer_init(int capacity)
{
	circular_buffer_t *circular_buf = (circular_buffer_t *)malloc(sizeof(circular_buffer_t));
	if(circular_buf == NULL)
	{
		return NULL;
	}

	circular_buf->data = (uint8_t *)malloc(capacity);
	if(circular_buf->data == NULL)
	{
		free(circular_buf);
		return NULL;
	}

	memset(circular_buf->data, 0, sizeof(circular_buf->data));
	
	circular_buf->size = 0;
	circular_buf->capacity = capacity;
	circular_buf->read = circular_buf->write = 0;
	
	return circular_buf;
}

void circular_buffer_uninit(circular_buffer_t *circular_buf)
{
	if(circular_buf == NULL)
	{
		return;
	}

	if(circular_buf->data != NULL)
	{
		free(circular_buf->data);
		circular_buf->data = NULL;
	}

	free(circular_buf);
}

void circular_buffer_reset(circular_buffer_t *circular_buf)
{
	if(circular_buf == NULL)
	{
		return;
	}

	circular_buf->read = 0;
	circular_buf->write = 0;
	circular_buf->size = 0;
}

int circular_buffer_read(circular_buffer_t *circular_buf, uint8_t *buf, int size)
{
	if(circular_buf == NULL || buf == NULL || size <= 0)
	{
		return 0;
	}

	int read_size = MIN(size, circular_buf->size);
	if(read_size <= (circular_buf->capacity - circular_buf->read))
	{
		memcpy(buf, circular_buf->data + circular_buf->read, read_size);
		
		circular_buf->read += read_size;
		
		if(circular_buf->read == circular_buf->capacity)
		{
			circular_buf->read = 0;
		}
	}
	else
	{
		int size1 = circular_buf->capacity - circular_buf->read;
		memcpy(buf, circular_buf->data + circular_buf->read, size1);
		
		int size2 = read_size - size1;
		memcpy(buf + size1, circular_buf->data, size2);
		
		circular_buf->read = size2;
	}

	circular_buf->size -= read_size;

	return read_size;
}

int circular_buffer_write(circular_buffer_t *circular_buf, const uint8_t *buf, int size)
{
	if(circular_buf == NULL || buf == NULL || size <= 0)
	{
		return 0;
	}

	int write_size = MIN(size, circular_buf->capacity - circular_buf->size);
	if(write_size <= circular_buf->capacity - circular_buf->write)
	{
		memcpy(circular_buf->data + circular_buf->write, buf, write_size);
		
		circular_buf->write += write_size;

		if(circular_buf->write == circular_buf->capacity)
		{
			circular_buf->write = 0;
		}
	}
	else
	{
		int size1 = circular_buf->capacity - circular_buf->write;
		memcpy(circular_buf->data + circular_buf->write, buf, size1);

		int size2 = write_size - size1;
		memcpy(circular_buf->data, buf + size1, size2);

		circular_buf->write = size2;
	}

	circular_buf->size += write_size;

	return write_size;
}

int circular_buffer_size(circular_buffer_t *circular_buf)
{
	return circular_buf->size;
}

int circular_buffer_capacity(circular_buffer_t *circular_buf)
{
	return circular_buf->capacity;
}

int circular_buffer_get_by_index(circular_buffer_t *circular_buf, int index, uint8_t *value)
{
	if(circular_buf == NULL || index < 0 || index >= circular_buf->size || value == NULL)
	{
		return -1;
	}

	index = (circular_buf->read + index) % circular_buf->capacity;

	*value = circular_buf->data[index];

	return 0;
}

void circular_buffer_pop(circular_buffer_t *circular_buf, const int n)
{
	if(circular_buf == NULL || n <= 0)
	{
		return;
	}
	
	int pop_size = MIN(n, circular_buf->size);
	if(pop_size <= circular_buf->capacity - circular_buf->read)
	{
		circular_buf->read += pop_size;
		if(circular_buf->read == circular_buf->capacity)
		{
			circular_buf->read = 0;
		}
	}
	else
	{
		int size1 = circular_buf->capacity - circular_buf->read;
		int size2 = pop_size - size1;
		circular_buf->read = size2;
	}

	circular_buf->size -= pop_size;
}










