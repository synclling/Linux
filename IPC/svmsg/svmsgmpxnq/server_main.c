#include "svmsg.h"

void server(int readfd, int writefd);

int main(int argc, char *argv[])
{
	int msqid;
	msqid = msgget(MQ_KEY1, SVMSG_MODE | IPC_CREAT);

	server(msqid, msqid);

	exit(0);
}
