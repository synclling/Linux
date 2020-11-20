#include "traceroute.h"

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

double time_difference(struct timeval begin, struct timeval end)
{
	struct timeval tv;
	tv.tv_sec = end.tv_sec - begin.tv_sec;
	tv.tv_usec = end.tv_usec - begin.tv_usec;

	if(tv.usec < 0)
	{
		tv.tv_sec--;
		tv.tv_usec += 1000000;
	}

	return tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
}



int main(int argc, char *argv[])
{
	if(argc != 2)
	{
		printf("usage: ./traceroute <IP address>\n");
		return -1;
	}

	char dest_str[128];			// 目的主机字符串
	memcpy(dest_str, argv[1], strlen(argv[1]) + 1);

	pid = getpid();				// 获取当前进程ID

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
	setsockopt(rawsock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)); // set time limit of socket's waiting for a packet

	int seq = 1;
	int recvcnt = 0;
	
	struct timeval begin, current;
	struct timeval sendtime[REQUEST_PER_TTL];
	
	for(int ttl = 1; ttl <= TTL_LIMIT; ++ttl)
	{
		setsockopt(rawsock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));

		for(int i = 0; i < REQUEST_PER_TTL; ++i)
		{
			icmp_pack(send_buf, seq++);

			gettimeofday(&sendtime[i], NULL);
			sendto(rawsock, sendbuf, ICMP_HEADER_LEN, 0, (struct sockaddr *)&addr, sizeof(addr));
		}

		gettimeofday(&begin, NULL); // get time after sending the packets

		while(recvcnt < REQUEST_PER_TTL)
		{
			int res = recvfrom(rawsock, recvbuf, BUFFER_SIZE, 0, 0, 0);
			if(res < 0)
			{
				gettimeofday(&current, NULL);
				if(time_difference(begin, current) > TIMEOUT) break;
				continue;
			}

			struct ip *iph = (struct ip *)recvbuf;
			if(iph->ip_p != IPPROTO_ICMP)
			{
				continue;
			}

			struct icmp *icmph = (struct icmp *)(recvbuf + iph->ip_hl * 4);
			
		}
	}
}
