#include "ipc.h"

void client(int readfd, int writefd);
void server(int readfd, int writefd);

int main(int argc, char *argv[])
{
	int res;
	
	pid_t childpid;				// child's pid
	int pipe1[2], pipe2[2];		// pipe1, pipe2
	
	res = pipe(pipe1);
	if(res < 0)
		err_sys("pipe error");

	res = pipe(pipe2);
	if(res < 0)
		err_sys("pipe error");

	childpid = fork();
	if(childpid == -1)
		err_sys("fork error");
	else if(childpid == 0) {	/* child */
		close(pipe1[1]);
		close(pipe2[0]);

		server(pipe1[0], pipe2[1]);

		exit(0);
	}

	/* parent */
	close(pipe1[0]);
	close(pipe2[1]);

	client(pipe2[0], pipe1[1]);

	waitpid(childpid, NULL, 0);

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
