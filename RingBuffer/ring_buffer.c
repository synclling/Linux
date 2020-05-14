#include "ring_buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <errno.h>

#include <sys/socket.h>
#include <sys/types.h>

static bool is_power_of_2(unsigned long n)
{
	return ((n != 0) && (n & (n - 1)) == 0);
}

static unsigned long roundup_power_of_two(unsigned long n)
{
	if(((n & (n - 1)) == 0)
	{
		return n;
	}

	unsigned long maxulong = (unsigned long)((unsigned long)~0);
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

	memset(ringbuf->data, 0, sizeof(ring->data));
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

unsigned int ringbuffer_read(struct ring_buffer *ringbuf, char *buf, unsigned int len)
{
	unsigned int readsize = min(len, ringbuf->write - ringbuf->read);

	
}

unsigned int ringbuffer_write(struct ring_buffer *ringbuf, char *buf, unsigned int len)
{
	
}

