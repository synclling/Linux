#include "mesg.h"

void server(int readfd, int writefd)
{
	FILE *fp;
	char *ptr;
	ssize_t n;
	pid_t pid;
	struct mymesg mesg;
	void sig_chld(int);

	signal(SIGCHLD, sig_chld);

	for(;;) {
		/* read pathname from our well-know queue */
		mesg.mesg_type = 1;
		if((n = mesg_recv(readfd, &mesg)) == 0) {
			err_msg("pathname missing");
			continue;
		}
		mesg.mesg_data[n] = '\0';	// null terminate pathname

		if((ptr = strchr(mesg.mesg_data, ' ')) == NULL) {
			err_msg("bogus request: %s", mesg.mesg_data);
			continue;
		}
		*ptr++ = 0;		// null terminate msqid, ptr = pathname
		writefd = atoi(mesg.mesg_data);
		
		if((pid = fork()) == -1)
			err_sys("fork error");
		else if(pid == 0) {
			if((fp = fopen(ptr, "r")) == NULL) {
				snprintf(mesg.mesg_data + n, sizeof(mesg.mesg_data) - n, ": can't open, %s\n", strerror(errno));
				mesg.mesg_len = strlen(ptr);
				memmove(mesg.mesg_data, ptr, mesg.mesg_len);
				mesg_send(writefd, &mesg);
			} else {
				/* fopen succeeded: copy file to client's queue */
				while(fgets(mesg.mesg_data, MAXMESGDATA, fp) != NULL) {
					mesg.mesg_len = strlen(mesg.mesg_data);
					mesg_send(writefd, &mesg);
				}
				fclose(fp);
			}

			/* send a 0-length message to signify the end */
			mesg.mesg_len = 0;
			mesg_send(writefd, &mesg);
			exit(0);
		}
		
	}
}
