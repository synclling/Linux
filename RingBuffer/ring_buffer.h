#ifndef __RINGBUFFER_H__
#define __RINGBUFFER_H__

#define min(x, y) ({\
	typeof(x) _x = (x);\
	typeof(y) _y = (y);\
	(void)(&_x == &_y);\
	_x < _y? _x : _y;})

struct ring_buffer
{
	void *data;
	unsigned int size;
	unsigned int read;
	unsigned int write;
};

struct ring_buffer *ringbuffer_create(unsigned int size);

unsigned int ringbuffer_read(struct ring_buffer *ringbuf, char *buf, unsigned int size);

unsigned int ringbuffer_write(struct ring_buffer *ringbuf, char *buf, unsigned int size);

unsigned int ringbuffer_length(struct ring_buffer *ringbuf);

unsigned int ringbuffer_empty(struct ring_buffer *ringbuf);

unsigned int ringbuffer_full(struct ring_buffer *ringbuf);

void ringbuffer_destroy(struct ring_buffer *ringbuf);

#endif