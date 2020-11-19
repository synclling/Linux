#include "traceroute.h"

uint16_t in_cksum(uint16_t *addr, int len, int csum)
{
	int sum = csum;

	while(len > 1)
	{
		sum += *addr++;
		len -= 2;
	}

	if(len == 1)
	{
		sum += htons(*(uint8_t)addr << 8);
	}

	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);

	return ~sum;
}


int main(int argc, char *argv[])
{
	if(argc != 2)
	{
		printf("usage: ./traceroute <IP address>\n");
		return -1;
	}

	char dest_str[128];				// 目的主机字符串
	memcpy(dest_str, argv[1], strlen(argv[1]) + 1);

	pid_t pid = getpid();			// 获取当前进程ID

	struct sockaddr_in addr;
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	inet_pton(AF_INET, dest_str, &addr.sin_addr);

	int rawsock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if(rawsock < 0)
	{
		perror("socket");
		return -2;
	}

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 1000;
	setsockopt(rawsock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
	
	for(int ttl = 1; ttl <= TTL_LIMIT; ++ttl)
	{
		
	}
}
