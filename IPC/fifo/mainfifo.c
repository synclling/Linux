#include "ipc.h"

#define FIFO1	"/tmp/fifo.1"
#define FIFO2	"/tmp/fifo.2"

void client(int readfd, int writefd);
void server(int readfd, int writefd);

int main(int argc, char *argv[])
{
	pid_t childpid;
	
	int readfd, writefd;

	/* mkfifo已隐含指定O_CREAT | O_EXCL      */
	if((mkfifo(FIFO1, FILE_MODE) < 0) && (errno != EEXIST))
		err_sys("can't create %s", FIFO1);

	if((mkfifo(FIFO2, FILE_MODE) < 0) && (errno != EEXIST)) {
		unlink(FIFO1);
		err_sys("can't create %s", FIFO2);
	}

	childpid = fork();
	if(childpid == -1)
		err_sys("fork error");
	else if(childpid == 0) {
		readfd = open(FIFO1, O_RDONLY, 0);
		writefd = open(FIFO2, O_WRONLY, 0);

		server(readfd, writefd);
		
		exit(0);
	}

	writefd = open(FIFO1, O_WRONLY, 0);
	readfd = open(FIFO2, O_RDONLY, 0);

	client(readfd, writefd);

	waitpid(childpid, NULL, 0);

	close(readfd);
	close(writefd);

	unlink(FIFO1);
	unlink(FIFO2);

	exit(0);
}


void client(int readfd, int writefd)
{
	size_t len;
	ssize_t n;

	char buf[MAXLINE];

	fgets(buf, MAXLINE, stdin);
	len = strlen(buf);
	if(buf[len - 1] == '\n')
		--len;

	write(writefd, buf, len);

	while((n = read(readfd, buf, MAXLINE)) > 0)
		write(STDOUT_FILENO, buf, n);
}

void server(int readfd, int writefd)
{
	int fd;		// file fd

	ssize_t n;
	char buf[MAXLINE + 1];

	if((n = read(readfd, buf, MAXLINE)) == 0)
		err_quit("end-of-file while reading pathname");
	buf[n] = '\0';

	if((fd = open(buf, O_RDONLY)) < 0) {
		snprintf(buf + n, sizeof(buf) - n, ": can't open, %s\n", strerror(errno));
		n = strlen(buf);
		write(writefd, buf, n);
	} else {
		while((n = read(fd, buf, MAXLINE)) > 0)
			write(writefd, buf, n);
		close(fd);
	}
}
