#include "svmsg.h"

void client(int readfd, int writefd);

int main(int argc, char *argv[])
{
	int readfd, writefd;

	/* server must create its well-know queue */
	writefd = msgget(MQ_KEY1, 0);

	/* we create our own private queue */
	readfd = msgget(IPC_PRIVATE, SVMSG_MODE | IPC_CREAT);

	client(readfd, writefd);

	msgctl(readfd, IPC_RMID, NULL); // delete our private queue
	
	exit(0);
}
