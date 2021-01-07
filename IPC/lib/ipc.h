#ifndef __IPC_H__
#define __IPC_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#define MAXLINE		4096

/* error.c */
void err_msg(const char *format, ...);
void err_ret(const char *format, ...);
void err_sys(const char *format, ...);
void err_dump(const char *format, ...);
void err_quit(const char *fromat, ...);


#endif
