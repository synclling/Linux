#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <fcntl.h> /* fcntl() */
#include <errno.h> /* errno */

#include <sys/socket.h> /* socket() */
#include <sys/select.h> /* select() */
#include <sys/time.h> /* struct timeval */
#include <netdb.h> /* gethostbyname() */
#include <arpa/inet.h> /* inet_ntoa() */

#include <sys/epoll.h>
#include <pthread.h>


#define HTTP_VERSION    "HTTP/1.1"
#define USER_AGENT      "User-Agent: Mozilla/5.0 (Windows NT 5.1; rv:10.0.2) Gecko/20100101 Firefox/10.0.2\r\n"
#define ENCODE_TYPE     "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
#define CONNECT_TYPE    "Connection: close\r\n"

#define BUFFER_SIZE 		4096
#define ASYNC_CLIENT_NUM	1024

struct http_request
{
	char *hostname;
	char *resource;
};

struct http_request reqs[] = 
{
	{ "api.seniverse.com", "/v3/weather/now.json?key=0pyd8z7jouficcil&location=beijing&language=zh-Hans&unit=c" },
	{ "api.seniverse.com", "/v3/weather/now.json?key=0pyd8z7jouficcil&location=changsha&language=zh-Hans&unit=c" },
	{ "api.seniverse.com", "/v3/weather/now.json?key=0pyd8z7jouficcil&location=shenzhen&language=zh-Hans&unit=c" },
	{ "api.seniverse.com", "/v3/weather/now.json?key=0pyd8z7jouficcil&location=shanghai&language=zh-Hans&unit=c" },
	{ "api.seniverse.com", "/v3/weather/now.json?key=0pyd8z7jouficcil&location=tianjin&language=zh-Hans&unit=c" },
	{ "api.seniverse.com", "/v3/weather/now.json?key=0pyd8z7jouficcil&location=wuhan&language=zh-Hans&unit=c" },
	{ "api.seniverse.com", "/v3/weather/now.json?key=0pyd8z7jouficcil&location=hefei&language=zh-Hans&unit=c" },
	{ "api.seniverse.com", "/v3/weather/now.json?key=0pyd8z7jouficcil&location=hangzhou&language=zh-Hans&unit=c" },
	{ "api.seniverse.com", "/v3/weather/now.json?key=0pyd8z7jouficcil&location=nanjing&language=zh-Hans&unit=c" },
	{ "api.seniverse.com", "/v3/weather/now.json?key=0pyd8z7jouficcil&location=jinan&language=zh-Hans&unit=c" },
	{ "api.seniverse.com", "/v3/weather/now.json?key=0pyd8z7jouficcil&location=taiyuan&language=zh-Hans&unit=c" },
	{ "api.seniverse.com", "/v3/weather/now.json?key=0pyd8z7jouficcil&location=wuxi&language=zh-Hans&unit=c" },
	{ "api.seniverse.com", "/v3/weather/now.json?key=0pyd8z7jouficcil&location=suzhou&language=zh-Hans&unit=c" }
};


char *http_host_to_ip(const char *hostname)
{
    struct hostent *host = gethostbyname(hostname);
    if(host == NULL)
    {
        return NULL;
    }
    
    return inet_ntoa(*(struct in_addr *)host->h_addr);
}

int http_async_client_commit(const char *hostname, const char *resource, int epfd)
{
	char *ip = http_host_to_ip(hostname);
	if(ip == NULL)
	{
		printf("error ip.\n");
		return -1;
	}
	
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
		printf("error socket.\n");
        return -1;
	}
	
	struct sockaddr_in serveraddr;
    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(ip);
    serveraddr.sin_port = htons(80);
	
	int ret = connect(sockfd, (struct sockaddr *)&serveraddr, sizeof(struct sockaddr_in));
	if(ret < 0)
	{
		printf("connect failed.\n");
        return -1;
	}

    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags |= O_NONBLOCK); // 设置套接字为非阻塞
	
	char buffer[BUFFER_SIZE] = {0};
	sprintf(buffer, "GET %s %s\r\nHost: %s\r\n%s\r\n", resource, HTTP_VERSION, hostname, CONNECT_TYPE);
	
	ret = send(sockfd, buffer, strlen(buffer), 0);
	if(ret < 0)
	{
		printf("send failed.\n");
		close(sockfd);
        return -1;
	}
	
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = sockfd;
	
	ret = epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);
	if(ret < 0)
	{
		printf("epoll_ctrl failed.\n");
		close(sockfd);
		return -1;
	}
	
	return 0;
}

void *http_async_client_recv(void *arg)
{
	int epfd = *(int *)arg;
	
	while(1)
	{
		struct epoll_event events[ASYNC_CLIENT_NUM] = {0};
		int ret = epoll_wait(epfd, events, ASYNC_CLIENT_NUM, -1);
		if(ret < 0)
		{
			if(errno == EINTR || errno == EAGAIN)
			{
				continue;
			}
			else
			{
				break;
			}
		}
		else if(ret == 0)
		{
			continue;
		}
		else
		{
			for(int i = 0; i < ret; ++i)
			{
				int sockfd = events[i].data.fd;
				char buff[BUFFER_SIZE] = {0};
				
				int size = recv(sockfd, buff, BUFFER_SIZE, 0);
				if(size > 0)
				{
					printf("%s\n\n", buff);
				}
				
				epoll_ctl(epfd, EPOLL_CTL_DEL, sockfd, NULL);
				
				close(sockfd);
			}
		}
	}
}

int http_async_client_request()
{
	int epfd = epoll_create(126);
	if(epfd < 0)
	{
		printf("epoll create failed.\n");
		return -1;
	}
	
	pthread_t pt;
	int ret = pthread_create(&pt, NULL, http_async_client_recv, &epfd);
	if(ret != 0)
	{
		printf("pthread create failed.\n");
		close(epfd);
		return -1;
	}
	
	usleep(1);
	
	int count = sizeof(reqs) / sizeof(reqs[0]);
	for(int i = 0; i < count; ++i)
	{
		http_async_client_commit(reqs[i].hostname, reqs[i].resource, epfd);
	}
	
	pthread_join(pt, NULL);
	
	close(epfd);
	
	return 0;
}


int main()
{
	http_async_client_request();
	return 0;
}