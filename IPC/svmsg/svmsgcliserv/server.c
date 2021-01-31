#include "mesg.h"

void server(int readfd, int writefd)
{
	FILE *fp;
	ssize_t n;
	struct mymesg	mesg;
	
	mesg.mesg_type = 1;
	if((n = mesg_recv(readfd, &mesg)) == 0)
		err_quit("pathname missing");
	mesg.mesg_data[n] = '\0';	// null terminate pathname

	if((fp = fopen(mesg.mesg_data, "r")) == NULL) {
		snprintf(mesg.mesg_data + n, sizeof(mesg.mesg_data) - n, ": can't open, %s\n", strerror(errno));
		mesg.mesg_len = strlen(mesg.mesg_data);
		mesg_send(writefd, &mesg);
	} else {
		/* for open succeeded: copy file to IPC channel */
		while(fgets(mesg.mesg_data, MAXMESGDATA, fp) != NULL) {
			mesg.mesg_len = strlen(mesg.mesg_data);
			mesg_send(writefd, &mesg);
		}
		fclose(fp);
	}

	/* for send a 0-length message to signify the end */
	mesg.mesg_len = 0;
	mesg_send(writefd, &mesg);
}
