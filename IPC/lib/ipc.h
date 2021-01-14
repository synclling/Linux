#ifndef __IPC_H__
#define __IPC_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define MAXLINE		4096

/* default permissions for new files */
#define FILE_MODE	(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
/* default permissions for new directories */
#define DIR_MODE	(FILE_MODE | S_IXUSR | S_IXGRP | S_IXOTH)

/* error.c */
void err_msg(const char *format, ...);
void err_ret(const char *format, ...);
void err_sys(const char *format, ...);
void err_dump(const char *format, ...);
void err_quit(const char *fromat, ...);


#endif
