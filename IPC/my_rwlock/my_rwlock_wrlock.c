#include "ipc.h"
#include "my_rwlock.h"

int my_rwlock_wrlock(my_rwlock_t *rw)
{
	if(rw->rw_magic != RW_MAGIC)
		return (EINVAL);

	int result;

	if((result = pthread_mutex_lock(&rw->rw_mutex)) != 0)
		return (result);

	while(rw->rw_refcount != 0) {
		rw->rw_nwaitwriters++;
		result = pthread_cond_wait(&rw->rw_condwriters, &rw->rw_mutex);
		rw->rw_nwaitwriters--;

		if(result != 0)
			break;
	}
	if(result == 0)
		rw->rw_refcount = -1;

	pthread_mutex_unlock(&rw->rw_mutex);
	return (result);
}

