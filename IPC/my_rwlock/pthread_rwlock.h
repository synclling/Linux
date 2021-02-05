#ifndef __PTHREAD_RWLOCK_H__
#define __PTHREAD_RWLOCK_H__

#include "ipc.h"

typedef struct {
	pthread_mutex_t		rw_mutex;			// basic lock
	pthread_cond_t		rw_condreaders;		// for reader threads waiting
	pthread_cond_t		rw_condwriters;		// for writer threads waiting
	int					rw_magic;			// for error checking
	int					rw_nwaitreaders;	// the number waiting
	int					rw_nwaitwriters;	// the number waiting
	int					rw_refcount;		// -1 if writer has the lock, else reader holding the lock
} pthread_rwlock_t;

typedef int		pthread_rwlockattr_t;

#define RW_MAGIC	0x19283746

#define PTHREAD_RWLOCK_INITIALIZER { PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER, RW_MAGIC, 0, 0, 0 }


int pthread_rwlock_init(pthread_rwlock_t *rw, pthread_rwlockattr_t *attr);
int pthread_rwlock_destroy(pthread_rwlock_t *rw);
int pthread_rwlock_rdlock(pthread_rwlock_t *rw);
int pthread_rwlock_wrlock(pthread_rwlock_t *rw);
int pthread_rwlock_tryrdlock(pthread_rwlock_t *rw);
int pthread_rwlock_trywrlock(pthread_rwlock_t *rw);
int pthread_rwlock_unlock(pthread_rwlock_t *rw);

#endif
