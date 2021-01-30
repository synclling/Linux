#include "ipc.h"


/* for want sizeof(struct mymesg) <= PIPE_BUF */
#define MAXMESGDATA	(PIPE_BUF - 2 * sizeof(long))
/* for length of mesg_len and mesg_type */
#define MESGHDRSIZE	(sizeof(struct mymesg) - MAXMESGDATA)

struct mymesg {
	long	mesg_len;					// #bytes in mesg_data, can be 0
	long	mesg_type;					// message type, must be > 0
	char	mesg_data[MAXMESGDATA];
};

ssize_t mesg_send(int id, struct mymesg *mptr)
{
	ssize_t n = msgsnd(id, &(mptr->mesg_type), mptr->mesg_len, 0);
	return (n);
}

ssize_t mesg_recv(int id, struct mymesg *mptr)
{
	ssize_t n = msgrcv(id, &(mptr->mesg_type), MAXMESGDATA, mptr->mesg_type, 0);
	mptr->mesg_len = n;

	return (n);
}
