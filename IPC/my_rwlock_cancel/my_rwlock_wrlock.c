#include "ipc.h"
#include "my_rwlock.h"

static void rwlock_cancelwrwait(void *arg)
{
	my_rwlock_t *rw;
	rw = (my_rwlock_t *)arg;

	rw->rw_nwaitwriters--;
	pthread_mutex_unlock(&rw->rw_mutex);
}

int my_rwlock_wrlock(my_rwlock_t *rw)
{
	if(rw->rw_magic != RW_MAGIC)
		return (EINVAL);

	int result;
	if((result = pthread_mutex_lock(&rw->rw_mutex)) != 0)
		return (result);

	while(rw->rw_refcount != 0) {
		rw->rw_nwaitwriters++;
		pthread_cleanup_push(rwlock_cancelwrwait, (void *)rw);
		result = pthread_cond_wait(&rw->rw_condwriters, &rw->rw_mutex);
		pthread_cleanup_pop(0);
		rw->rw_nwaitwriters--;
		if(result != 0)
			break;
	}
	if(result == 0)
		rw->rw_refcount = -1;

	pthread_mutex_unlock(&rw->rw_mutex);
	return (0);
}
