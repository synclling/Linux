#ifndef __RINGBUFFER_H__
#define __RINGBUFFER_H__

#define min(x, y) ({ (x) < (y)? (x) : (y) })

struct ring_buffer
{
	void *data;
	unsigned int size;
	unsigned int read;
	unsigned int write;
};

struct ring_buffer *ringbuffer_create(unsigned int size);

void ringbuffer_destroy(struct ring_buffer *ringbuf);

#endif