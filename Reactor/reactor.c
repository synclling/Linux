

#define BUFFER_SIZE 4096
#define MAX_EPOLL_EVENTS 1024

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

void nty_event_set(struct ntyevent* ev, int sockfd, int events, NCALLBACK cb, void *arg)
{
	ev->sockfd = sockfd;
	ev->events = events;
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

int ntyreactor_add_event(struct ntyreactor *reactor, struct ntyevent *ev)
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

int ntyreactor_del_event(struct ntyreactor *reactor, struct ntyevent *ev)
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

int ntyreactor_run(struct ntyreactor *reactor)
{
	if(reactor == NULL)
	{
		return -1;
	}
	if(reactor->epfd < 0 || reactor->events == NULL)
	{
		return -1;
	}
	
	while(1)
	{
		
	}
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
		return -3
	}
	
	return sockfd;
}

int recv_cb(int sockfd, int events, void *arg)
{
	struct ntyreactor * reactor = (struct ntyreactor *)arg;
	
	
}

int send_cb(int sockfd, int events, void *arg)
{
	
}






















