#include "svmsg.h"

void client(int readfd, int writefd);

int main(int argc, char *argv[])
{
	int readid, writeid;

	writeid = msgget(MQ_KEY1, 0);
	readid = msgget(MQ_KEY2, 0);

	client(readid, writeid);

	msgctl(readid, IPC_RMID, NULL);
	msgctl(writeid, IPC_RMID, NULL);

	exit(0);
}