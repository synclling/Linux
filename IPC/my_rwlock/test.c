#include "ipc.h"
#include "my_rwlock.h"

#define MAXNTHREADS		100


struct {
	my_rwlock_t			rwlock;			// read-write lock
	pthread_mutex_t		rcountlock;		// protect nreaders
	int	nreaders;
	int nwriters;
} shared = { MY_RWLOCK_INITIALIZER, PTHREAD_MUTEX_INITIALIZER };

int nloop = 1000;
int nreaders = 6;
int nwriters = 4;

void *reader(void *arg);
void *writer(void *arg);


int main(int argc, char *argv[])
{
	int c, i;
	pthread_t tid_readers[MAXNTHREADS], tid_writers[MAXNTHREADS];

	while((c = getopt(argc, argv, "n:r:w:")) != -1) {
		switch(c) {
			case 'n':
				nloop = atoi(optarg);
				break;
			case 'r':
				nreaders = atoi(optarg);
				break;
			case 'w':
				nwriters = atoi(optarg);
				break;
		}
	}

	if(optind != argc)
		err_quit("usage: test [-n #loops] [-r #readers] [-w #writers]");

	/* create all the reader and writer threads */
	for(i = 0; i < nreaders; ++i)
		pthread_create(&tid_readers[i], NULL, reader, NULL);
	for(i = 0; i < nwriters; ++i)
		pthread_create(&tid_writers[i], NULL, writer, NULL);

	/* wait for all the threads to complete */
	for(i = 0; i < nreaders; ++i)
		pthread_join(tid_readers[i], NULL);
	for(i = 0; i < nwriters; ++i)
		pthread_join(tid_writers[i], NULL);

	exit(0);
}

void *reader(void *arg)
{
	int i;
	for(i = 0; i < nloop; ++i) {
		my_rwlock_rdlock(&shared.rwlock);

		pthread_mutex_lock(&shared.rcountlock);
		shared.nreaders++;
		pthread_mutex_unlock(&shared.rcountlock);

		if(shared.nwriters > 0)
			err_quit("reader: %d writers found", shared.nwriters);

		pthread_mutex_lock(&shared.rcountlock);
		shared.nreaders--;
		pthread_mutex_unlock(&shared.rcountlock);

		my_rwlock_unlock(&shared.rwlock);
	}
	return (NULL);
}

void *writer(void *arg)
{
	int i;
	for(i = 0; i < nloop; ++i) {
		my_rwlock_wrlock(&shared.rwlock);

		shared.nwriters++;		// only one writer, need not protect

		if(shared.nwriters > 1)
			err_quit("writer: %d writers found", shared.nwriters);
		if(shared.nreaders > 0)
			err_quit("writer: %d readers found", shared.nreaders);

		shared.nwriters--;
	
		my_rwlock_unlock(&shared.rwlock);
	}
	return (NULL);
}

