#include "traceroute.h"

int gotalarm = 0;

void signal_alarm(int signo)
{
	gotalarm = 1;
	return;
}

uint16_t icmp_cksum(uint16_t *data, int len)
{
	uint32_t sum = 0;

	while(len > 1)
	{
		sum += *data++;
		len -= 2;
	}

	if(len == 1)
	{
		uint8_t tmp = *(uint8_t *)data;
		sum += tmp;
	}

	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);

	return ~sum;
}

void icmp_pack(struct icmp *icmph, int seq)
{
	icmph->icmp_type = ICMP_ECHO;
	icmph->icmp_code = 0;
	icmph->icmp_cksum = 0;
	icmph->icmp_id = pid & 0xffff;
	icmph->icmp_seq = seq & 0xffff;

	icmp->icmp_cksum = icmp_cksum((uint16_t *)icmph, ICMP_HEADER_LEN); // 此icmp报文没有数据，故只校验头部长度
}


int icmp_recv(int rawsock, struct timeval *recvtime)
{
	gotalarm = 0;
	alarm(3);

	for(;;)
	{
		if(gotalarm == 1)
			return (-3);

		int size = recvfrom(rawsock, send_buf, BUFFER_SIZE, 0, NULL, NULL);
		if(size < 0)
		{
			if(errno == EINTR)
				continue;
			else
				perror("recv error");
		}

		struct ip *iph = (struct ip *)recvbuf;
		if(iph->ip_p != IPPROTO_ICMP)
		{
			continue;
		}

		struct icmp *icmph = (struct icmp *)(recvbuf + iph->ip_hl * 4);
		if(icmph->icmp_type == ICMP_TIME_EXCEEDED)
		{
				
		}
		else if(icmph->icmp_type == ICMP_ECHOREPLY && icmph->icmp_id == pid)
		{
			struct ip *icmp_ip = (struct ip *)icmph->icmp_data;
			icmph = (struct icmp *)(icmph->icmp_data + icmp_ip->ip_hl * 4);
			if(icmph->icmp_id != pid)
			{
				continue;
			}

			totaltime += time_difference(sendtime[(icmph->icmp_seq - 1) % REQUEST_PER_TTL], currnet);
		}
	}
}


double icmp_tvsub(struct timeval start, struct timeval end)
{
	return (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;
}




int main(int argc, char *argv[])
{
	if(argc != 2)
	{
		printf("usage:./traceroute <address>\n");
		return -1;
	}

	pid = getpid();					// 获取当前进程ID
	signal(SIGALRM, signal_alrm)	// 注册信号处理函数

	struct sockaddr_in addr;		// 地址结构
	bzero(&addr, sizeof(addr));
	
	addr.sin_family = AF_INET;
	inet_pton(AF_INET, argv[1], &addr.sin_addr);

	// 原始套接字
	int rawsock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if(rawsock < 0)
	{
		perror("socket");
		return -2;
	}

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 1000;
	setsockopt(rawsock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)); // set time limit of socket's waiting for a packet

	int seq = 1;
	
	struct timeval sendtime;
	struct timeval recvtime;

	for(int ttl = 1; ttl <= TTL_LIMIT; ++ttl)
	{
		int replies = 0;
		
		setsockopt(rawsock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));
		
		for(int i = 0; i < PACKETS_PER_HOP; ++i)
		{
			icmp_pack(send_buf, seq++);
			gettimeofday(&sendtime, NULL);
			sendto(rawsock, send_buf, ICMP_HEADER_LEN, 0, (struct sockaddr *)&addr, sizeof(addr));

			int res = icmp_recv(rawsock, &recvtime);
		}

	}
	
	
	for(int ttl = 1; ttl <= TTL_LIMIT; ++ttl)
	{
		setsockopt(rawsock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));

		for(int i = 0; i < REQUEST_PER_TTL; ++i)
		{
			icmp_pack(send_buf, seq++);

			gettimeofday(&sendtime[(seq - 1) % REQUEST_PER_TTL], NULL);
			sendto(rawsock, sendbuf, ICMP_HEADER_LEN, 0, (struct sockaddr *)&addr, sizeof(addr));
		}

		gettimeofday(&begin, NULL); // get time after sending the packets

		while(recvcnt < REQUEST_PER_TTL)
		{
			int res = recvfrom(rawsock, recvbuf, BUFFER_SIZE, 0, 0, 0);
			if(res < 0)
			{
				continue;
			}

			gettimeofday(&current, NULL);

			struct ip *iph = (struct ip *)recvbuf;
			if(iph->ip_p != IPPROTO_ICMP)
			{
				continue;
			}

			struct icmp *icmph = (struct icmp *)(recvbuf + iph->ip_hl * 4);
			if(icmph->icmp_type == ICMP_TIME_EXCEEDED)
			{
				
			}
			else if(icmph->icmp_type == ICMP_ECHOREPLY && icmph->icmp_id == pid)
			{
				struct ip *icmp_ip = (struct ip *)icmph->icmp_data;
				icmph = (struct icmp *)(icmph->icmp_data + icmp_ip->ip_hl * 4);
				if(icmph->icmp_id != pid)
				{
					continue;
				}

				totaltime += time_difference(sendtime[(icmph->icmp_seq - 1) % REQUEST_PER_TTL], currnet);
			}
			
		}
	}
}
