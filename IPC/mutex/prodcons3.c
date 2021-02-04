#include "ipc.h"

#define MAXNITEMS		1000000
#define MAXNTHREADS		100

int nitems;		// read-only by producer and consumer

struct {
	pthread_mutex_t mutex;
	int buf[MAXNITEMS];
	int nput;
	int nval;
} shared = { PTHREAD_MUTEX_INITIALIZER };

void *produce(void *arg);
void *consume(void *arg);

int main(int argc, char *argv[])
{
	int i, nthreads, count[MAXNTHREADS];
	pthread_t tid_produce[MAXNTHREADS], tid_consume;

	if(argc != 3)
		err_quit("usage: prodcons3 <#items> <#threads>");

	nitems = min(atoi(argv[1]), MAXNITEMS);
	nthreads = min(atoi(argv[2]), MAXNTHREADS);

	for(i = 0; i < nthreads; ++i) {
		count[i] = 0;
		pthread_create(&tid_produce[i], NULL, produce, &count[i]);
	}

	pthread_create(&tid_consume, NULL, consume, NULL);

	for(i = 0; i < nthreads; ++i) {
		pthread_join(tid_produce[i], NULL);
		printf("count[%d] = %d\n", i, count[i]);
	}

	pthread_join(tid_consume, NULL);

	exit(0);
}

void *produce(void *arg)
{
	for(;;) {
		pthread_mutex_lock(&shared.mutex);
		if(shared.nput >= nitems) {
			pthread_mutex_unlock(&shared.mutex);
			return (NULL);
		}
		shared.buf[shared.nput] = shared.nval;
		shared.nput++;
		shared.nval++;
		pthread_mutex_unlock(&shared.mutex);
		*((int *)arg) += 1;
	}
}

void consume_wait(int i)
{
	for(;;) {
		pthread_mutex_lock(&shared.mutex);
		if(i < shared.nput) {
			pthread_mutex_unlock(&shared.mutex);
			return;		// an item is ready
		}
		pthread_mutex_unlock(&shared.mutex);
	}
}

void *consume(void *arg)
{
	int i;
	for(i = 0; i < nitems; ++i) {
		consume_wait(i);
		if(shared.buf[i] != i)
			printf("buf[%d] = %d\n", i, shared.buf[i]);
	}
	return (NULL);
}
