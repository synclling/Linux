#include "svmsg.h"

void client(int readfd, int writefd);

int main(int argc, char *argv[])
{
	int msqid;
	msqid = msgget(MQ_KEY1, 0);

	client(msqid, msqid);

	exit(0);
}
