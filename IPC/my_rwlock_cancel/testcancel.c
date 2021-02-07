#include "ipc.h"
#include "my_rwlock.h"


pthread_t tid1, tid2;

my_rwlock_t  rwlock = MY_RWLOCK_INITIALIZER;


void *thread1(void *arg);
void *thread2(void *arg);

int main(int argc, char *argv[])
{
	void *status;

	my_rwlock_init(&rwlock, NULL);

	pthread_create(&tid1, NULL, thread1, NULL);

	sleep(1);

	pthread_create(&tid2, NULL, thread2, NULL);
	pthread_join(tid2, &status);
	if(status != PTHREAD_CANCELED)
		printf("thread2 status = %p\n", status);

	pthread_join(tid1, &status);
	if(status != NULL)
		printf("thread1 status = %p\n", status);

	printf("rw_refcount = %d, rw_nwaitreaders = %d, rw_nwaitwriters = %d\n", 
		rwlock.rw_refcount, rwlock.rw_nwaitreaders, rwlock.rw_nwaitwriters);

	my_rwlock_destroy(&rwlock);

	exit(0);
}

void *thread1(void *arg)
{
	my_rwlock_rdlock(&rwlock);
	printf("thread1() got a read lock\n");
	sleep(3);
	pthread_cancel(tid2);
	sleep(3);
	my_rwlock_unlock(&rwlock);
	return (NULL);
}

void *thread2(void *arg)
{
	printf("thread2() trying to obtain, a write lock\n");
	my_rwlock_wrlock(&rwlock);
	printf("thread2() got a write lock\n");
	sleep(1);
	my_rwlock_unlock(&rwlock);
	return (NULL);
}
