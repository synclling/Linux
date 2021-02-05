#include "my_rwlock.h"

int my_rwlock_destroy(my_rwlock_t *rw)
{
	if(rw->rw_magic != RW_MAGIC)
		return (EINVAL);

	if(rw->rw_nwaitreaders != 0 || rw->rw_nwaitwriters != 0 || rw->rw_refcount != 0)
		return (EBUSY);

	pthread_mutex_destroy(&rw->rw_mutex);
	pthread_cond_destroy(&rw->rw_condreaders);
	pthread_cond_destroy(&rw->rw_condwriters);
	rw->rw_refcount = 0;

	return (0);
}
