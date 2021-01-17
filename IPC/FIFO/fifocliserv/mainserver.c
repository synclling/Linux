#include "fifo.h"

int main(int argc, char *argv[])
{
	int readfifo, writefifo, dummyfd, fd;
	char *ptr, buf[MAXLINE], fifoname[MAXLINE];
	pid_t pid;
	ssize_t n;

	if((mkfifo(SERV_FIFO, FILE_MODE) < 0) && (errno != EEXIST))
		err_sys("can't create %s", SERV_FIFO);

	readfifo = open(SERV_FIFO, O_RDONLY, 0);
	dummyfd = open(SERV_FIFO, O_WRONLY, 0); 	// dummyfifo never used

	while((n = readline(readfifo, buf, MAXLINE)) > 0) {
		if(buf[n - 1] == '\n')
			--n;			// delete newline from readline
		buf[n] = '\0';		// null terminate pathname

		if((ptr = strchr(buf, ' ')) == NULL) {
			err_msg("bogus request: %s", buf);
			continue;
		}

		*ptr++ = 0;			// null terminate PID, ptr = pathname
		pid = atol(buf);

		snprintf(fifoname, sizeof(fifoname), "/tmp/fifo.%ld", (long)pid);
		if((writefifo = open(fifoname, O_WRONLY, 0)) < 0) {
			err_msg("can not open: %s", fifoname);
			continue;
		}

		if((fd = open(ptr, O_RDONLY)) < 0) {
			snprintf(buf + n, sizeof(buf) - n, ": can't open, %s\n", strerror(errno));
			n = strlen(ptr);
			write(writefifo, ptr, n);
			close(writefifo);
		} else {
			while((n = read(fd, buf, MAXLINE)) > 0)
				write(writefifo, buf, n);
			close(fd);
			close(writefifo);
		}
	}
}
