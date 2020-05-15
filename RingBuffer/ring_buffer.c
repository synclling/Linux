#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <unistd.h>
#include <errno.h>

#include <sys/socket.h>
#include <sys/types.h>

#include "ring_buffer.h"

static bool is_power_of_2(unsigned long n)
{
	return ((n != 0) && (n & (n - 1)) == 0);
}

static unsigned long roundup_power_of_two(unsigned long n)
{
	if((n & (n - 1)) == 0)
	{
		return n;
	}

	unsigned long maxulong = (unsigned long)~0;
	unsigned long andv = ~(maxulong & (maxulong >> 1));

	while((andv & n) == 0)
	{
		andv = andv >> 1;
	}

	return andv << 1;
}


struct ring_buffer *ringbuffer_create(unsigned int size)
{
	struct ring_buffer *ringbuf = NULL;

	if(!is_power_of_2(size))
	{
		size = roundup_power_of_two(size);
	}

	ringbuf = (struct ring_buffer *)malloc(sizeof(struct ring_buffer));
	if(ringbuf == NULL)
	{
		perror("malloc error.\n");
		return NULL;
	}

	ringbuf->data = (void *)malloc(size);
	if(ringbuf->data == NULL)
	{
		perror("malloc error.\n");
		free(ringbuf);
		return NULL;
	}

	memset(ringbuf->data, 0, sizeof(ringbuf->data));
	ringbuf->size = size;
	ringbuf->read = ringbuf->write = 0;

	return ringbuf;
}

void ringbuffer_destroy(struct ring_buffer *ringbuf)
{
	if(ringbuf != NULL)
	{
		free(ringbuf->data);
		ringbuf->data = NULL;
		
		free(ringbuf);
	}
}

unsigned int ringbuffer_read(struct ring_buffer *ringbuf, char *buf, unsigned int size)
{
	unsigned int len;
	
	size = min(size, ringbuf->write - ringbuf->read);
	len = min(size, ringbuf->size - (ringbuf->read & (ringbuf->size - 1)));

	/* first get the data from ring_buf->read_pos until the end of the buffer */
	memcpy(buf, ringbuf->data + (ringbuf->read & (ringbuf->size - 1)), len);

	/* then get the rest (if any) from the beginning of the buffer */
	memcpy(buf + len, ringbuf->data, size - len);

	ringbuf->read += size;

	return size;
}

unsigned int ringbuffer_write(struct ring_buffer *ringbuf, char *buf, unsigned int size)
{
	unsigned int len;

	size = min(size, ringbuf->size - (ringbuf->write - ringbuf->read));
	len = min(size, ringbuf->size - (ringbuf->write & (ringbuf->size - 1)));

	/* first put the data starting from write_pos to buffer end */
	memcpy(ringbuf->data + (ringbuf->write & (ringbuf->size - 1)), buf, len);

	/* then put the rest (if any) at the beginning of the buffer */
	memcpy(ringbuf->data, buf + len, size - len);

	ringbuf->write += size;

	return size;
}

unsigned int ringbuffer_length(struct ring_buffer *ringbuf)
{
	return (ringbuf->write - ringbuf->read);
}


unsigned int ringbuffer_empty(struct ring_buffer *ringbuf)
{
	return (ringbuf->write == ringbuf->read);
}

unsigned int ringbuffer_full(struct ring_buffer *ringbuf)
{
	return ((ringbuf->write - ringbuf->read) == ringbuf->size);
}

