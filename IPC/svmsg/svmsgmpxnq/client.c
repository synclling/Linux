#include "mesg.h"

void client(int readfd, int writefd)
{
	size_t len;
	ssize_t n;
	char *ptr;
	struct mymesg mesg;

	/* start buffer with msqid and a blank */
	snprintf(mesg.mesg_data, MAXMESGDATA, "%d ", readfd);
	len = strlen(mesg.mesg_data);
	ptr = mesg.mesg_data + len;

	/* read pathname */
	fgets(ptr, MAXMESGDATA - len, stdin);
	len = strlen(mesg.mesg_data);
	if(mesg.mesg_data[len - 1] == '\n')
		--len;
	mesg.mesg_len = len;
	mesg.mesg_type = 1;

	/* write msqid and pathname to server's well-know queue */
	mesg_send(writefd, &mesg);

	/* read from our queue, write to standard output */
	while((n = mesg_recv(readfd, &mesg)) > 0)
		write(STDOUT_FILENO, mesg.mesg_data, n);
}
