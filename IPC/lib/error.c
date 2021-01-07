#include "ipc.h"

#include <stdarg.h>

static void msg_print(int errnoflag, const char *format, va_list ap);

/* Nonfatal error unrelated to a system call.
 * Print a message and return. */
void err_msg(const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	msg_print(0, format, ap);
	va_end(ap);
	
	return;
}

/* Nonfatal error related to a system call.
 * Print a message and return. */
void err_ret(const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	msg_print(1, format, ap);
	va_end(ap);
	
	return;
}

/* Fatal error related to a system call.
 * Print a message and terminate. */
void err_sys(const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	msg_print(1, format, ap);
	va_end(ap);

	exit(1);
}

/* Fatal error related to a system call.
 * Print a message, dump core, and terminate. */
void err_dump(const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	msg_print(1, format, ap);
	va_end(ap);

	abort();	// dump core and terminate.
	exit(1);	// should't get here.
}

/* Fatal error unrelated to a system call.
 * Print a message and terminate. */
void err_quit(const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	msg_print(0, format, ap);
	va_end(ap);

	exit(1);
}

/* Print a message and return to caller.
 * Caller specifies "errnoflag". */
static void msg_print(int errnoflag, const char *format, va_list ap)
{
	int errno_save;			// save errno;
	char buf[MAXLINE];
	
	errno_save = errno;		// value caller might want printed.

	vsnprintf(buf, sizeof(buf), format, ap);

	int len = strlen(buf);
	if(errnoflag)
	{
		snprintf(buf + len, sizeof(buf) - len, ": %s\n", strerror(errno_save));
	}

	fflush(stdout);		// in case stdout and stderr are the same.
	fputs(buf, stderr);
	fflush(stderr);

	return;
}
