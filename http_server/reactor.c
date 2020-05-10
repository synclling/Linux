#include "reactor.h"

int accept_cb(int sockfd, void *arg)
{
	ntyreactor *reactor = (ntyreactor *)arg;
	if(reactor == NULL)
	{
		return -1;
	}

	struct sockaddr_in clientaddr;
	socklen_t len = sizeof(struct sockaddr_in);

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

	printf("new connection [%s:%d][time:%ld]\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port), reactor->events[clientfd].last_active);

	return 0;
}

int recv_cb(int sockfd, void *arg)
{
	struct ntyreactor *reactor = (struct ntyreactor *)arg;
	struct ntyevent *ev = &reactor->events[sockfd];

	ntyreactor_del_event_from_epoll(reactor, ev);

	int size = recv(sockfd, ev->recvbuf, BUFFER_SIZE, 0);
	if(size > 0)
	{
		ev->recvsize = size;
		ev->recvbuf[size] = '\0';

		printf("recv[sockfd=%d]: %s\n", sockfd, ev->recvbuf);

		reactor->httphandler(ev); // 处理http请求包

		ntyevent_set(ev, sockfd, EPOLLOUT, send_cb, (void *)reactor);
		ntyreactor_add_event_to_epoll(reactor, ev);
	}
	else if(size == 0)
	{
		close(sockfd);
		printf("[sockfd=%d] closed.\n", sockfd);
	}
	else
	{
		close(sockfd);
		printf("recv[sockfd=%d] error[%d]: %s\n", sockfd, errno, strerror(errno));
	}

	return size;
}

int send_cb(int sockfd, void *arg)
{
	struct ntyreactor *reactor = (struct ntyreactor *)arg;
	struct ntyevent *ev = &reactor->events[sockfd];

	ntyreactor_del_event_from_epoll(reactor, ev);

	int size = send(sockfd, ev->sendbuf, BUFFER_SIZE, 0);
	if(size > 0)
	{
		printf("send[sockfd=%d]: %s\n", sockfd, ev->sendbuf);

		ntyevent_set(ev, sockfd, EPOLLIN, recv_cb, (void *)reactor);
		ntyreactor_add_event_to_epoll(reactor, ev);
	}
	else
	{
		close(sockfd);
		printf("send[sockfd=%d] error[%d]: %s\n", sockfd, errno, strerror(errno));
	}

	return size;
}



void ntyevent_set(struct ntyevent* ev, int sockfd, int events, NCALLBACK cb, void *arg)
{
	ev->sockfd = sockfd;
	ev->events = events; 	// EPOLLIN or EPOLLOUT
	ev->status = 0;			// init 0
	ev->last_active = time(NULL);
	
	ev->arg = arg;
	ev->cb = cb;
}

int init_socket(int port)
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
		return -1;
	}

	int flags = fcntl(sockfd, F_GETFL, 0);
	fcntl(sockfd, F_SETFL, flags |= O_NONBLOCK);

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
	
	if(ev->status == 0)
	{
		struct epoll_event epollev;
		epollev.events = ev->events;
		epollev.data.ptr = (void *)ev;
		
		int res = epoll_ctl(reactor->epfd, EPOLL_CTL_ADD, ev->sockfd, &epollev);
		if(res < 0)
		{
			return -2;
		}
		
		ev->status = 1; // set status
	}
	
	return 0;
}

int ntyreactor_del_event_from_epoll(struct ntyreactor *reactor, struct ntyevent *ev)
{
	if(reactor == NULL || ev == NULL)
	{
		return -1;
	}
	
	if(ev->status == 1)
	{
		struct epoll_event epollev;
		epollev.events = ev->events;
		epollev.data.ptr = (void *)ev;
		
		int res = epoll_ctl(reactor->epfd, EPOLL_CTL_DEL, ev->sockfd, &epollev);
		if(res < 0)
		{
			return -2;
		}
		
		ev->status = 0;
	}
	
	return 0;
}

int ntyreactor_add_listener(struct ntyreactor *reactor, int sockfd, NCALLBACK accept_cb)
{
	if(reactor == NULL || reactor->events == NULL)
	{
		return -1;
	}

	ntyevent_set(&reactor->events[sockfd], sockfd, EPOLLIN, accept_cb, (void *)reactor);
	ntyreactor_add_event_to_epoll(reactor, ev);

	return 0;
}

int ntyreactor_run(struct ntyreactor *reactor)
{
}

int ntyreactor_destory(struct ntyreactor *reactor)
{
}

