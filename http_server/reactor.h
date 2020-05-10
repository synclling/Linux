#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#define BUFFER_SIZE 4096
#define MAX_EPOLL_EVENTS 1024
#define SERVER_PORT	8888

typedef int (*NCALLBACK)(int sockfd, void *arg);
typedef int (*NHTTPHANDLER)(void *arg);

struct ntyevent
{
	int sockfd;		// socket
	int events;		// EPOLLIN or EPOLLOUT
	int status;		// if the ntyevent has been added to ntyreactor then status = 1, else status = 0

	void *arg;	
	NCALLBACK cb;	// callback

	long last_active;
	
	int recvsize;	// recv buffer size
	int sendsize;	// send buffer size
	
	char recvbuf[BUFFER_SIZE];	// recv buffer
	char sendbuf[BUFFER_SIZE];	// send buffer
};

struct ntyreactor
{
	int epfd;
	struct ntyevent *events;
	NHTTPHANDLER httphandler;
};




int accept_cb(int sockfd, void *arg);
int recv_cb(int sockfd, void *arg);
int send_cb(int sockfd, void *arg);


int ntyreactor_init(struct ntyreactor *reactor);

int ntyreactor_add_event_to_epoll(struct ntyreactor *reactor, struct ntyevent *ev);

int ntyreactor_del_event_from_epoll(struct ntyreactor *reactor, struct ntyevent *ev);

int ntyreactor_add_listener(struct ntyreactor *reactor, int sockfd, NCALLBACK accept_cb);

int ntyreactor_run(struct ntyreactor *reactor);

int ntyreactor_destory(struct ntyreactor *reactor);

