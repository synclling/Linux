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

#define BUFFER_SIZE 4096

#define HTTP_VERSION    "HTTP/1.1"
#define USER_AGENT      "User-Agent: Mozilla/5.0 (Windows NT 5.1; rv:10.0.2) Gecko/20100101 Firefox/10.0.2\r\n"
#define ENCODE_TYPE     "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
#define CONNECT_TYPE    "Connection: close\r\n"

#if 0

char *http_host_to_ip(const char *hostname)
{
    struct hostent *host = gethostbyname(hostname);
    if(host == NULL)
    {
        return NULL;
    }
    
    return inet_ntoa(*(struct in_addr *)host->h_addr);
}

int http_create_socket(const char *ip)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
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
        return -1;
    }

    //fcntl(sockfd, F_SETFL, O_NONBLOCK);

    return sockfd;
}

int http_send_request(int sockfd, const char *hostname, const char *resource)
{
    char buffer[BUFFER_SIZE] = {0};
    sprintf(buffer, "GET %s %s\r\nHost: %s\r\n%s\r\n", resource, HTTP_VERSION, hostname, CONNECT_TYPE);
    printf("buffer: %s", buffer);

    send(sockfd, buffer, strlen(buffer), 0);

    memset(buffer, '\0', BUFFER_SIZE);
    recv(sockfd, buffer, BUFFER_SIZE, 0);
    printf("recv: %s\n", buffer);

    return 0;
}

int http_client_commit(const char *hostname, const char *resource)
{
    char *ip = http_host_to_ip(hostname);
	if(ip == NULL)
	{
		printf("ip error.\n");
		return -1;
	}
	
    int sockfd = http_create_socket(ip);
	if(sockfd < 0)
	{
		printf("socket error.\n");
		return -1;
	}
	
    http_send_request(sockfd, hostname, resource);

    close(sockfd);
}

int main(int argc, char *argv[])
{
    http_client_commit(argv[1], argv[2]);

    return 0;
}


#else

struct http_request
{
	char *hostname;
	char *resource;
	int sockfd;
};

struct http_request reqs[] = {
	{"api.seniverse.com", "/v3/weather/now.json?key=0pyd8z7jouficcil&location=beijing&language=zh-Hans&unit=c", 0 },
	{"api.seniverse.com", "/v3/weather/now.json?key=0pyd8z7jouficcil&location=changsha&language=zh-Hans&unit=c", 0 },
	{"api.seniverse.com", "/v3/weather/now.json?key=0pyd8z7jouficcil&location=shenzhen&language=zh-Hans&unit=c", 0 },
	{"api.seniverse.com", "/v3/weather/now.json?key=0pyd8z7jouficcil&location=shanghai&language=zh-Hans&unit=c", 0 },
	{"api.seniverse.com", "/v3/weather/now.json?key=0pyd8z7jouficcil&location=tianjin&language=zh-Hans&unit=c", 0 },
	{"api.seniverse.com", "/v3/weather/now.json?key=0pyd8z7jouficcil&location=wuhan&language=zh-Hans&unit=c", 0 },
	{"api.seniverse.com", "/v3/weather/now.json?key=0pyd8z7jouficcil&location=hefei&language=zh-Hans&unit=c", 0 },
	{"api.seniverse.com", "/v3/weather/now.json?key=0pyd8z7jouficcil&location=hangzhou&language=zh-Hans&unit=c", 0 },
	{"api.seniverse.com", "/v3/weather/now.json?key=0pyd8z7jouficcil&location=nanjing&language=zh-Hans&unit=c", 0 },
	{"api.seniverse.com", "/v3/weather/now.json?key=0pyd8z7jouficcil&location=jinan&language=zh-Hans&unit=c", 0 },
	{"api.seniverse.com", "/v3/weather/now.json?key=0pyd8z7jouficcil&location=taiyuan&language=zh-Hans&unit=c", 0 },
	{"api.seniverse.com", "/v3/weather/now.json?key=0pyd8z7jouficcil&location=wuxi&language=zh-Hans&unit=c", 0 },
	{"api.seniverse.com", "/v3/weather/now.json?key=0pyd8z7jouficcil&location=suzhou&language=zh-Hans&unit=c", 0 },
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

int http_create_socket(const char *hostname)
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

    fcntl(sockfd, F_SETFL, O_NONBLOCK); // 设置套接字为非阻塞

    return sockfd;
}

int http_client_commit()
{
	int fd_max = 0;
	int sockfd = -1;
	
	fd_set readset;
	FD_ZERO(&readset);
	
	struct timeval tv;
	tv.tv_sec = 5;
	tv.tv_usec = 0;
	
    int count = sizeof(reqs) / sizeof(reqs[0]);
	for(int i = 0; i < count; ++i)
	{
		sockfd = http_create_socket(reqs[i].hostname);
		if(sockfd < 0)
		{
			for(int j = 0; j < i; ++j)
			{
				close(reqs[j].sockfd);
			}
			return -1;
		}
		
		reqs[i].sockfd = sockfd;
		FD_SET(reqs[i].sockfd, &readset);
		
		if(reqs[i].sockfd > fd_max)
		{
			fd_max = reqs[i].sockfd;
		}
		
		char buffer[BUFFER_SIZE] = {0};
		sprintf(buffer, "GET %s %s\r\nHost: %s\r\n%s\r\n", reqs[i].resource, HTTP_VERSION, reqs[i].hostname, CONNECT_TYPE);

		send(reqs[i].sockfd, buffer, strlen(buffer), 0);
	}
	
	while(1)
	{
		int res = select(fd_max + 1, &readset, NULL, NULL, &tv);
		if(res < 0) // 失败
		{
			if(errno == EINTR)
			{
				continue;
			}
			else
			{
				break;
			}
		}
		else if(res == 0) // 超时
		{
			continue;
		}
		else
		{
			for(int i = 0; i < count; ++i)
			{
				if(FD_ISSET(reqs[i].sockfd, &readset))
				{
					memset(buffer, '\0', BUFFER_SIZE);
					recv(reqs[i].sockfd, buffer, BUFFER_SIZE, 0);
					printf("%s\n\n", buffer);
				}
			}
		}
	}
	
	for(int i = 0; i < count; ++i)
	{
		close(reqs[i].sockfd);
	}
    
	return 0;
}



int main(int argc, char *argv[])
{
	http_client_commit();

    return 0;
}
#endif