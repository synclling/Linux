#include "ipc.h"

#define MQ_KEY1		1234L
#define MQ_KEY2		2345L

void server(int readfd, int writefd);

int main(int argc, char *argv[])
{
	int readid, writeid;

	readid = msgget(MQ_KEY1, SVMSG_MODE | IPC_CREAT);
	if(readid == -1)
		err_sys("msgget error");
	writeid = msgget(MQ_KEY2, SVMSG_MODE | IPC_CREAT);
	if(writeid == -1)
		err_sys("msgget error");

	server(readid, writeid);

	exit(0);
}

