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
} my_rwlock_t;

typedef int		my_rwlockattr_t;			// not supported

#define RW_MAGIC	0x19283746

#define MY_RWLOCK_INITIALIZER { PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER, RW_MAGIC, 0, 0, 0 }


int my_rwlock_init(my_rwlock_t *rw, my_rwlockattr_t *attr);
int my_rwlock_destroy(my_rwlock_t *rw);
int my_rwlock_rdlock(my_rwlock_t *rw);
int my_rwlock_wrlock(my_rwlock_t *rw);
int my_rwlock_tryrdlock(my_rwlock_t *rw);
int my_rwlock_trywrlock(my_rwlock_t *rw);
int my_rwlock_unlock(my_rwlock_t *rw);

#endif
