#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <time.h>
#include <fcntl.h>  /* fcntl() */

#include <sys/socket.h> /* socket() */
#include <netdb.h>  /* gethostbyname() */
#include <arpa/inet.h>  /* inet_ntoa() */

#define BUFFER_SIZE 4096

#define HTTP_VERSION    "HTTP/1.1"
#define USER_AGENT      "User-Agent: Mozilla/5.0 (Windows NT 5.1; rv:10.0.2) Gecko/20100101 Firefox/10.0.2\r\n"
#define ENCODE_TYPE     "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
#define CONNECT_TYPE    "Connection: close\r\n"

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
    if(sockfd == -1)
    {
        perror("create socket error.");
        return -1;
    }

    struct sockaddr_in serveraddr;
    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(ip);
    serveraddr.sin_port = htons(80);

    int ret = connect(sockfd, (struct sockaddr *)&serveraddr, sizeof(struct sockaddr_in));
    if(ret == -1)
    {
        perror("connect error.");
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
    printf("ip: %s\n", ip);
    int sockfd = http_create_socket(ip);
    http_send_request(sockfd, hostname, resource);

    close(sockfd);
}

int main(int argc, char *argv[])
{
    http_client_commit(argv[1], argv[2]);

    return 0;
}
