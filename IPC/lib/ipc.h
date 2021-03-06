#ifndef __IPC_H__
#define __IPC_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>			/* for nonblocking */
#include <limits.h>			/* PIPE_BUF */
#include <signal.h>
#include <time.h>			/* timespec{} for pselect() */
#include <sys/time.h>		/* timeval{} for select() */
#include <sys/types.h>		/* basic system data types */
#include <sys/stat.h>		/* for S_xxx file mode constants */
#include <sys/wait.h>

#include <sys/ipc.h>		/* System V IPC */
#include <sys/msg.h>		/* System V message queues */

#include <pthread.h>


#define MAXLINE		4096

/* default permissions for new files */
#define FILE_MODE	(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

/* default permissions for new directories */
#define DIR_MODE	(FILE_MODE | S_IXUSR | S_IXGRP | S_IXOTH)

/* default permissions for new System V message queues */
#define MSG_R		0400
#define MSG_W		0200
#define SVMSG_MODE	(MSG_R | MSG_W | MSG_R >> 3 | MSG_R >> 6)



#define min(a, b) ((a) < (b)? (a) : (b))
#define max(a, b) ((a) > (b)? (a) : (b))


/* error.c */
void err_msg(const char *format, ...);
void err_ret(const char *format, ...);
void err_sys(const char *format, ...);
void err_dump(const char *format, ...);
void err_quit(const char *fromat, ...);


#endif
