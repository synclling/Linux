#include "fifo.h"

int main(int argc, char *argv[])
{
	int readfifo, writefifo;
	size_t len;
	ssize_t n;
	char *ptr, fifoname[MAXLINE], buf[MAXLINE];
	pid_t pid;

	// create FIFO with our PID as part of name
	pid = getpid();
	snprintf(fifoname, sizeof(fifoname), "/tmp/fifo.%ld", (long)pid);
	if((mkfifo(fifoname, FILE_MODE) < 0) && (errno != EEXIST))
		err_sys("can't create %s", fifoname);

	// start buf with pid and a blank
	snprintf(buf, sizeof(buf), "%ld ", (long)pid);
	len = strlen(buf);
	ptr = buf + len;

	// read pathname
	fgets(ptr, MAXLINE - len, stdin);	// fgets() guarantees null byte at the end
	len = strlen(buf);

	// open FIFO to server and write PID and pathname to FIFO
	writefifo = open(SERV_FIFO, O_WRONLY, 0);
	write(writefifo, buf, len);

	// open our FIFO, blocks until server opens for writing
	readfifo = open(fifoname, O_RDONLY, 0);

	// read from IPC, write to standard output
	while((n = read(readfifo, buf, MAXLINE)) > 0)
		write(STDOUT_FILENO, buf, n);

	close(readfifo);
	unlink(fifoname);

	exit(0);
}
