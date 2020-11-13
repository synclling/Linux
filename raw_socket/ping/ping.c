#include "ping.h"


/* 终端信号处理函数 */
static void icmp_sigint(int signo)
{
	alive = 0;

	gettimeofday(&tv_end, NULL);
	tv_internal = icmp_tvsub(tv_begin, tv_end);
}

static unsigned short icmp_cksum(unsigned char *data, int len)
{
	int sum = 0;				// 校验和
	int odd = len & 0x01;		// 是否为奇数

	// 将数据按照2字节为单位累加起来
	while(len & 0xfffe)
	{
		sum += *(unsigned short *)data;
		data += 2;
		len -= 2;
	}

	// 若ICMP报头为奇数个字节，会剩下最后一个字节
	if(odd)
	{
		unsigned short tmp = ((*data) << 8) & 0xff00;
		sum += tmp;
	}

	sum = (sum >> 16) + (sum & 0xffff); 	// 高低位相加
	sum += (sum >> 16);						// 将溢出位加入

	return ~sum;	// 返回取反值
}

static void icmp_pack(struct icmp *icmph, int seq, int len)
{
	unsigned char i = 0;
	
	icmph->icmp_type = ICMP_ECHO;	// ICMP回显请求
	icmph->icmp_code = 0;			// ICMP回显请求的code值为0
	icmph->icmp_cksum = 0;			// 先将cksum值填写0，便于之后的cksum的计算
	icmph->icmp_seq = seq;			// 本报文的序列号
	icmph->icmp_id = pid & 0xffff;	// 填写进程PID

	for(i = 0; i < len; ++i)
		icmph->icmp_data[i] = i;

	// 计算校验和
	icmph->icmp_cksum = icmp_cksum((unsigned char *)icmph, len);
}


static int icmp_unpack(char *buf, int len)
{
	int ip_head_len = 0;		// IP头部长度
	
	struct ip *ip = NULL;		// IP首部结构
	struct icmp *icmp = NULL;	// ICMP首部结构

	ip = (struct ip *)buf;
	ip_head_len = ip->ip_hl * 4; // IP首部长度是以4个字节为单位

	icmp = (struct icmp *)(buf + ip_head_len);
	len -= ip_head_len;

	if(len < 8)
	{
		printf("ICMP packet's length is less than 8.\n");
		return -1;
	}

	// 判断ICMP类型为ICMP_ECHOREPLY并且是本进程的PID
	if((icmp->icmp_type == ICMP_ECHOREPLY) && (icmp->icmp_id == pid))
	{
		struct timeval tv_send, tv_recv, tv_internal;

		ping_packet *packet = icmp_findpacket(icmp->icmp_seq);
		if(packet == NULL)
		{
			return -1;
		}

		packet->flag = 0;	// 已收到该包的响应包

		tv_send = packet->tv_send;
		gettimeofday(&tv_recv, NULL);
		tv_internal = icmp_tvsub(tv_send, tv_recv);

		int rtt = tv_internal.tv_sec * 1000 + tv_internal.tv_usec / 1000;

		// 打印结果，ICMP包长度，源IP地址，包的序列号，TTL，时间差
		printf("%d bytes from %s: icmp_seq = %u ttl = %d rtt = %d ms\n", 
			len, inet_ntoa(ip->ip_src), icmp->icmp_seq, ip->ip_ttl, rtt);

		++packet_recv;
	}
	else
	{
		return -1;
	}

	return 0;
}


static void *icmp_send(void *argv)
{
	gettimeofday(&tv_begin, NULL);		// 开始发送的时间

	while(alive)
	{
		int size = 0;
		
		ping_packet *pack = icmp_findpacket(-1); // 在数组中找到一个空闲包位置
		if(pack != NULL)
		{
			pack->seq = packet_send;
			pack->flag = 1;
			gettimeofday(&pack->tv_send, NULL); // 发送时间
		}

		icmp_pack((struct icmp *)send_buf, packet_send, 64); // 打包数据

		size = sendto(rawsock, send_buf, 64, 0, (struct sockaddr *)&dest, sizeof(dest));
		if(size < 0)
		{
			perror("sendto error");
			continue;
		}

		++packet_send;

		sleep(1);	// 每隔1s发送一个ICMP回显请求包
	}
}


static void *icmp_recv(void *argv)
{
	fd_set readfd;
	
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 200;

	while(alive)
	{
		int ret = 0;
		
		FD_ZERO(&readfd);
		FD_SET(rawsock, &readfd);

		ret = select(rawsock + 1, &readfd, NULL, NULL, &tv);
		switch (ret)
		{
			case -1:
				/* 发生错误 */
				break;
			case 0:
				/* 超时 */
				break;
			default:
			{
				int size = recv(rawsock, recv_buf, sizeof(recv_buf), 0);
				if(errno == EINTR)
				{
					perror("recv from error");
					continue;
				}

				ret = icmp_unpack(recv_buf, size);
				if(ret == -1)
				{
					continue;
				}
			}
			break;
		}
	}
}


static struct timeval icmp_tvsub(struct timeval begin, struct timeval end)
{
	struct timeval tv;

	tv.tv_sec = end.tv_sec - begin.tv_sec;
	tv.tv_usec = end.tv_usec - begin.tv_usec;

	// 若接收时间的usec值小于发送时间的usec值，则从sec借位
	if(tv.tv_usec < 0)
	{
		tv.tv_sec--;
		tv.tv_usec += 1000000;
	}

	return tv;
}


static void icmp_statistics()
{
	long time = tv_internal.tv_sec * 1000 + tv_internal.tv_usec / 1000;
	printf("--- %s ping statistics ---\n", dest_str);
	printf("%d packets transmitted, %d received, %d%% packet loss, time %ldms\n", 
		packet_send, packet_recv, (packet_send - packet_recv) * 100 / packet_send, time);
}

static ping_packet *icmp_findpacket(int seq)
{
	int i = 0;
	ping_packet *found = NULL;

	if(seq >= 0)			// 查找对应seq的包
	{
		for(i = 0; i < 128; ++i)
		{
			if(seq == packets[i].seq)
			{
				found = &packets[i];
				break;
			}
		}
	}
	else if(seq == -1)		// 查找空包的位置
	{
		for(i = 0; i < 128; ++i)
		{
			if(packets[i].flag == 0)
			{
				found = &packets[i];
				break;
			}
		}
	}

	return found;
}

int main(int argc, char *argv[])
{
	struct hostent *host = NULL;
	struct protoent *protocol = NULL;

	char protoname[] = "icmp";
	unsigned long inaddr = 1;
	int size = 128 * 1024;

	if(argc < 2)
	{
		printf("ping aaa.bbb.ccc.ddd\n");
		return -1;
	}

	protocol = getprotobyname(protoname);
	if(protocol == NULL)
	{
		perror("getprotobyname()");
		return -1;
	}

	memcpy(dest_str, argv[1], strlen(argv[1]) + 1);
	memset(packets, 0, sizeof(struct ping_packet) * 128);

	rawsock = socket(AF_INET, SOCK_RAW, protocol->p_proto);
	if(rawsock < 0)
	{
		perror("socket");
		return -1;
	}

	pid = getuid();

	// 增大接收缓冲区，防止接收的包被覆盖
	setsockopt(rawsock, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));

	bzero(&dest, sizeof(dest));
	dest.sin_family = AF_INET;

	inaddr = inet_addr(argv[1]);
	if(inaddr == INADDR_NONE)
	{
		host = gethostbyname(argv[1]);
		if(host == NULL)
		{
			perror("gethostbyname");
			return -1;
		}

		memcpy((char *)&dest.sin_addr, host->h_addr, host->h_length);
	}
	else
	{
		memcpy((char *)&dest.sin_addr, &inaddr, sizeof(inaddr));
	}

	inaddr = dest.sin_addr.s_addr;

	printf("PING %s (%ld.%ld.%ld.%ld) 56(84) bytes of data.\n", dest_str, 
		(inaddr & 0x000000ff) >> 0, 
		(inaddr & 0x0000ff00) >> 8, 
		(inaddr & 0x00ff0000) >> 16, 
		(inaddr & 0xff000000) >> 24);

	signal(SIGINT, icmp_sigint);

	alive = 1;

	pthread_t send_id, recv_id;

	int err = 0;
	err = pthread_create(&send_id, NULL, icmp_send, NULL);
	if(err < 0)
	{
		return -1;
	}
	err = pthread_create(&recv_id, NULL, icmp_recv, NULL);
	if(err < 0)
	{
		return -1;
	}

	pthread_join(send_id, NULL);
	pthread_join(recv_id, NULL);

	close(rawsock);

	icmp_statistics();

	return 0;
	
}
