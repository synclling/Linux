#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#include "ring_buffer.h"

#define FIFO_SIZE 16

void *consumer_proc(void *arg)
{
	struct ring_buffer *ringbuf = (struct ring_buffer *)arg;

	char i;
	unsigned int cnt = 0;
	while(1)
	{
		sleep(1);
		printf("----------------------------------\n");
		printf("    get data from ring buffer.\n");

		if(ringbuffer_empty(ringbuf))
		{
			printf("    buffer is empty.\n");
			sleep(1);
			continue;
		}

		if(cnt != 0 && !(cnt % FIFO_SIZE))
		{
			printf("\n");
		}

		ringbuffer_read(ringbuf, &i, sizeof(i));
		printf("    data is: %d\n", i);

		++cnt;

		printf("    ring buffer length: %u\n", ringbuffer_length(ringbuf));
		printf("----------------------------------\n\n");
	}
}

void *producer_proc(void *arg)
{
	struct ring_buffer *ringbuf = (struct ring_buffer *)arg;

	char i = 0;
	while(1)
	{
		sleep(1);
		printf("**********************************\n");
		printf("    put data to ring buffer.\n");

		if(ringbuffer_full(ringbuf))
		{
			printf("    buffer is full.\n");
			sleep(1);
			continue;
		}

		ringbuffer_write(ringbuf, &i, sizeof(i));
		++i;

		printf("    ring buffer length: %u\n", ringbuffer_length(ringbuf));
		printf("**********************************\n\n");
	}
}

pthread_t consumer_thread(void *arg)
{
	pthread_t pid;
	
	int res = pthread_create(&pid, NULL, consumer_proc, arg);
	if(res != 0)
	{
		fprintf(stderr, "Failed to create consumer thread.[errno:%u, reason:%s]\n", errno, strerror(errno));
		return -1;
	}

	return pid;
}

pthread_t producer_thread(void *arg)
{
	pthread_t pid;
	
	int res = pthread_create(&pid, NULL, producer_proc, arg);
	if(res != 0)
	{
		fprintf(stderr, "Failed to create producer thread.[errno:%u, reason:%s]\n", errno, strerror(errno));
		return -1;
	}

	return pid;
}

int main()
{
	pthread_t consume_pid;
	pthread_t produce_pid;
	
	struct ring_buffer *ringbuf = ringbuffer_create(FIFO_SIZE);
	if(ringbuf == NULL)
	{
		perror("ringbuffer_create()");
		exit(1);
	}

	printf("multi thread test...\n");

	produce_pid = producer_thread((void *)ringbuf);
	consume_pid = consumer_thread((void *)ringbuf);

	pthread_join(produce_pid, NULL);
	pthread_join(consume_pid, NULL);

	ringbuffer_destroy(ringbuf);

	return 0;
}