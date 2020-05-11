#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>

#include <sys/mman.h>
#include <sys/stat.h>

#include "reactor.h"

enum
{
	HTTP_RESP_NOT_FOUND = 404,
	HTTP_RESP_SERVER_UNAVAILABLE = 503
};

int bad_request(char *buffer, const char *header, const char *htmlpath)
{
	struct stat st;
	int res = stat(htmlpath, &st);
	if(res < 0)
	{
		return -1;
	}

	// header
	int size = 0;
	strcpy(buffer, header);
	size += strlen(header);

	const char *content_type = "Content-Type: text/html;charset=utf-8\r\n";
	strcat(buffer + size, content_type);
	size += strlen(content_type);

	char content_length[256] = {0};
	sprintf(content_length, "Content-Length: %ld\r\n", st.st_size);
	
	strcat(buffer + size, content_length);
	size += strlen(content_length);

	strcat(buffer + size, "\r\n");
	size += 2;

	// body
	int fd = open(htmlpath, O_RDONLY);
	char *body = (char *)mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
	strcat(buffer + size, body);
	size += st.st_size;

	close(fd);

	return size;
}

int send_errno(char *buffer, int errcode)
{
	switch (errcode)
	{
		case HTTP_RESP_NOT_FOUND:
			return bad_request(buffer, "HTTP/1.0 404 Not Found\r\n", "www/404.html");
		case HTTP_RESP_SERVER_UNAVAILABLE:
			return bad_request(buffer, "HTTP/1.0 503 Server Unavailable\r\n", "www/503.html");
	}
}

int http_handler(void *arg)
{
	struct ntyevent *ev = (struct ntyevent *)arg;

	int size = ev->recvsize;
	char *recvbuf = ev->recvbuf;

	ev->sendsize = send_errno(ev->sendbuf, HTTP_RESP_NOT_FOUND);
	
	return 0;
}

int main()
{

	ntyreactor_setup(http_handler);

	return 0;
}
