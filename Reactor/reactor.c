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

typedef int (*NCALLBACK)(int sockfd, int events, void *arg);

struct ntyevent
{
	int sockfd; // socket
	int events;	// EPOLLIN or EPOLLOUT
	int flag;	// if the ntyevent has been added to ntyreactor then flag = 1, else flag = 0
	long last_active;
	
	NCALLBACK cb;
	void *arg;
	
	int size;
	char buffer[BUFFER_SIZE];
};

struct ntyreactor
{
	int epfd;
	struct ntyevent *events;
};

int accept_cb(int sockfd, int events, void *arg);
int recv_cb(int sockfd, int events, void *arg);
int send_cb(int sockfd, int events, void *arg);

void ntyevent_set(struct ntyevent* ev, int sockfd, int events, NCALLBACK cb, void *arg)
{
	ev->sockfd = sockfd;
	ev->events = events; // EPOLLIN or EPOLLOUT
	ev->flag = 0;	// init 0
	ev->last_active = time(NULL);
	
	ev->arg = arg;
	ev->cb = cb;
}

int ntyreactor_init(struct ntyreactor *reactor)
{
	if(reactor == NULL)
	{
		return -1;
	}
	
	memset(reactor, 0, sizeof(struct ntyreactor));
	
	reactor->epfd = epoll_create(1);
	if(reactor->epfd < 0)
	{
		printf("create epfd in %s error: %s\n", __func__, strerror(errno));
		return -2;
	}
	
	reactor->events =  (struct ntyevent *)malloc(MAX_EPOLL_EVENTS * sizeof(struct ntyevent));
	if(reactor->events == NULL)
	{
		printf("malloc in %s error: %s\n", __func__, strerror(errno));
		close(reactor->epfd);
		return -3;
	}
	
	return 0;
}

int ntyreactor_add_event_to_epoll(struct ntyreactor *reactor, struct ntyevent *ev)
{
	if(reactor == NULL || ev == NULL)
	{
		return -1;
	}
	
	if(ev->flag == 0)
	{
		struct epoll_event epollev;
		epollev.events = ev->events;
		epollev.data.ptr = (void *)ev;
		
		int res = epoll_ctl(reactor->epfd, EPOLL_CTL_ADD, ev->sockfd, &epollev);
		if(res < 0)
		{
			return -2;
		}
		
		ev->flag = 1; // set flag
	}
	
	return 0;
}

int ntyreactor_del_event_from_epoll(struct ntyreactor *reactor, struct ntyevent *ev)
{
	if(reactor == NULL || ev == NULL)
	{
		return -1;
	}
	
	if(ev->flag == 1)
	{
		struct epoll_event epollev;
		epollev.events = ev->events;
		epollev.data.ptr = (void *)ev;
		
		int res = epoll_ctl(reactor->epfd, EPOLL_CTL_DEL, ev->sockfd, &epollev);
		if(res < 0)
		{
			return -2;
		}
		
		ev->flag = 0;
	}
	
	return 0;
}

int ntyreactor_add_listener(struct ntyreactor *reactor, int sockfd, NCALLBACK accept_cb)
{
	if(reactor == NULL || reactor->events == NULL)
	{
		return -1;
	};
	
	ntyevent_set(&reactor->events[sockfd], sockfd, EPOLLIN, accept_cb, (void *)reactor);
	ntyreactor_add_event_to_epoll(reactor, &reactor->events[sockfd]);
	
	return 0;
}

int ntyreactor_run(struct ntyreactor *reactor)
{
	if(reactor == NULL || reactor->epfd < 0 || reactor->events == NULL)
	{
		return -1;
	}
	
	struct epoll_event events[MAX_EPOLL_EVENTS];
	
	while(1)
	{
		int nfds = epoll_wait(reactor->epfd, events, MAX_EPOLL_EVENTS, 1000);
		if(nfds < 0)
		{
			continue;
		}
		
		for(int i = 0; i < nfds; ++i)
		{
			struct ntyevent *ev = (struct ntyevent *)events[i].data.ptr;
			if((events[i].events & EPOLLIN) && (ev->events & EPOLLIN))
			{
				ev->cb(ev->sockfd, events[i].events, ev->arg);
			}
			
			if((events[i].events & EPOLLOUT) && (ev->events & EPOLLOUT))
			{
				ev->cb(ev->sockfd, events[i].events, ev->arg);
			}
		}
	}
	
	return 0;
}

int ntyreactor_destory(struct ntyreactor *reactor)
{
	if(reactor == NULL)
	{
		return -1;
	}
	
	close(reactor->epfd);
	free(reactor->events);
	
	return 0;
}

int init_socket(int port)
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
		return -1;
	}
	
	int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags |= O_NONBLOCK); // 设置套接字为非阻塞
	
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(struct sockaddr_in));
	serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htonl(port);
	
	int res = bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(struct sockaddr));
	if(res < 0)
	{
		printf("bind failed: %s\n", strerror(errno));
		return -2;
	}
	
	res = listen(sockfd, 10);
	if(res < 0)
	{
		printf("listen failed: %s\n", strerror(errno));
		return -3;
	}
	
	return sockfd;
}

int accept_cb(int sockfd, int events, void *arg)
{
	struct ntyreactor *reactor = (struct ntyreactor *)arg;
	if(reactor == NULL)
	{
		return -1;
	}
	
	struct sockaddr_in clientaddr;
	memset(&clientaddr, 0, sizeof(struct sockaddr_in));

    socklen_t len = sizeof(clientaddr);
	int clientfd = accept(sockfd, (struct sockaddr *)&clientaddr, &len);
	if(clientfd < 0)
	{
		printf("accept failed: %s\n", strerror(errno));
		return -2;
	}
	
	int flags = fcntl(clientfd, F_GETFL, 0);
    fcntl(clientfd, F_SETFL, flags |= O_NONBLOCK); // 设置套接字为非阻塞
	
	ntyevent_set(&reactor->events[clientfd], clientfd, EPOLLIN, recv_cb, (void *)reactor);
	ntyreactor_add_event_to_epoll(reactor, &reactor->events[clientfd]);
	
	return 0;
}

int recv_cb(int sockfd, int events, void *arg)
{
	struct ntyreactor *reactor = (struct ntyreactor *)arg;
	struct ntyevent *ev = &reactor->events[sockfd];
	
	int size = recv(sockfd, ev->buffer, BUFFER_SIZE, 0);
	
	ntyreactor_del_event_from_epoll(reactor, ev);
	
	if(size > 0)
	{
		ev->size = size;
		ev->buffer[size] = '\0';
		
		printf("Client[%d]:%s\n", sockfd, ev->buffer);
		
		ntyevent_set(ev, sockfd, EPOLLOUT, send_cb, (void *)reactor);
		ntyreactor_add_event_to_epoll(reactor, ev);
	}
	else if(size == 0)
	{
		close(sockfd);
		printf("[sockfd=%d] pos[%ld], closed\n", sockfd, ev - reactor->events);
	}
	else
	{
		close(sockfd);
		printf("recv[sockfd=%d] error[%d]:%s\n", sockfd, errno, strerror(errno));
	}
	
	return size;
}

int send_cb(int sockfd, int events, void *arg)
{
	struct ntyreactor *reactor = (struct ntyreactor *)arg;
	struct ntyevent *ev = &reactor->events[sockfd];
	
	int size = send(sockfd, ev->buffer, ev->size, 0);
	if(size > 0)
	{
		printf("send[sockfd=%d], [%d]%s\n", sockfd, size, ev->buffer);

		ntyreactor_del_event_from_epoll(reactor, ev);
		ntyevent_set(ev, sockfd, EPOLLIN, recv_cb, (void *)reactor);
		ntyreactor_add_event_to_epoll(reactor, ev);
	}
	else
	{
		close(sockfd);
		ntyreactor_del_event_from_epoll(reactor, ev);
		printf("send[sockfd=%d] error %s\n", sockfd, strerror(errno));
	}
	
	return size;
}

int main(int argc, char *argv[])
{
	int sockfd = init_socket(SERVER_PORT);
	
	struct ntyreactor reactor;
	ntyreactor_init(&reactor);
	
	ntyreactor_add_listener(&reactor, sockfd, accept_cb);
	
	ntyreactor_run(&reactor);
	
	ntyreactor_destory(&reactor);
	
	close(sockfd);
	
	return 0;
}




















