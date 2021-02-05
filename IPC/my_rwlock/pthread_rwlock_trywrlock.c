#include "pthread_rwlock.h"

int pthread_rwlock_trywrlock(pthread_rwlock_t *rw)
{
	if(rw->rw_magic != RW_MAGIC)
		return (EINVAL);

	int result;

	if((result = pthread_mutex_lock(&rw->rw_mutex)) != 0)
		return (result);

	if(rw->rw_refcount != 0)
		result = EBUSY;
	else
		rw->rw_refcount = -1;

	pthread_mutex_unlock(&rw->rw_mutex);
	return (result);
}

