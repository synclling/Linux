#include "mesg.h"

void server(int readfd, int writefd)
{
	FILE *fp;
	char *ptr;
	pid_t pid;
	ssize_t n;
	struct mymesg mesg;

	for(;;) {
		/* read pathname from IPC channel */
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

		*ptr++ = 0;		// null terminate PID, ptr = pathname
		pid = atol(mesg.mesg_data);
		mesg.mesg_type = pid;

		if((fp = fopen(ptr, "r")) == NULL) {
			snprintf(mesg.mesg_data + n, sizeof(mesg.mesg_data) - n, ": can't open, %s\n", strerror(errno));
			mesg.mesg_len = strlen(ptr);
			memmove(mesg.mesg_data, ptr, mesg.mesg_len);
			mesg_send(writefd, &mesg);
		} else {
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
}
