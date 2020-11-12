#include "ping.h"

static int rawsock = 0;					// 套接字
static char dest_str[128];				// 目的主机字符串
static struct sockaddr_in dest;			// 目的地址
static char send[SEND_BUFFER_SIZE];		// 发送缓冲区
static char recv[RECV_BUFFER_SIZE];		// 接收缓冲区
static short packet_send = 0;			// 已经发送的数据包数量
static short packet_recv = 0;			// 已经接收的数据包数量
static struct timeval tv_begin;			// 本程序开始发送的时间
static struct timeval tv_end;			// 本程序结束发送的时间
static struct timeval tv_internal;		// 本程序开始到结束的时间间隔

static pid_t pid;						// 进程ID
static int alive = 0;					// 是否接收到退出信号

static ping_packet packets[128];		// 保存ICMP包


static unsigned short icmp_cksum(unsigned char *data, int len)
{
	int sum = 0;				// 计算结果
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
	
	struct ip *ip = NULL;		// IP头部结构
	struct icmp *icmp = NULL;	// ICMP头部结构

	ip = (struct ip *)buf;
	ip_head_len = ip->ip->hl * 4;

	icmp = (struct icmp *)(buf + ip_head_len);
	len -= ip_head_len;

	if(len < 8)
	{
		printf("ICMP packet's length is less than 8.\n");
		return -1;
	}

	if((icmp->icmp_type == ICMP_ECHOREPLY) && (icmp->icmp_id == pid))
	{
		
	}
}


static void *icmp_send(void *argv);


static void *icmp_recv(void *argv);


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
	
}


int main(int argc, char *argv[])
{
	
}
