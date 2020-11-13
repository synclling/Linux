#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <pthread.h>
#include <signal.h>
#include <errno.h>

#include <sys/time.h>
#include <sys/socket.h>

#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>


#define SEND_BUFFER_SIZE 	72		// 发送缓冲区的大小
#define RECV_BUFFER_SIZE 	2048	// 接收缓冲区的大小

/* 保存已发送包的状态 */
typedef struct ping_packet {
	short 	seq;					// 该包的序列号
	int 	flag;					// 1表示已发送但没有收到响应包，0表示收到响应包
	struct timeval tv_send;			// 发送该包的时间
} ping_packet;

static int rawsock = 0;					// 套接字
static char dest_str[128];				// 目的主机字符串
static struct sockaddr_in dest;			// 目的地址
static char send_buf[SEND_BUFFER_SIZE];	// 发送缓冲区
static char recv_buf[RECV_BUFFER_SIZE];	// 接收缓冲区
static short packet_send = 0;			// 已经发送的ICMP包数量
static short packet_recv = 0;			// 已经接收的ICMP包数量
static struct timeval tv_begin;			// 本程序开始发送的时间
static struct timeval tv_end;			// 本程序结束发送的时间
static struct timeval tv_internal;		// 本程序开始到结束的时间间隔

static pid_t pid;						// 进程ID
static int alive = 0;					// 是否接收到退出信号
static ping_packet packets[128];		// ICMP包数组，保存已经发送的ICMP包

/*
 *	@brief	CRC16校验和计算，校验包括ICMP报文的首部和数据部分
 *	@param	[in]data		ICMP报文
 *	@param	[in]len			ICMP报文总长度
 *	@return 返回校验和
 */
static unsigned short icmp_cksum(unsigned char *data, int len);

/*
 *	@brief	设置ICMP报头
 *	@param	[in]icmph		ICMP报头
 *	@param	[in]seq			序列号
 *  @param  [in]len			长度
 *	@return 
 */
static void icmp_pack(struct icmp *icmph, int seq, int len);

/*
 *	@brief	解压接收到的数据报
 *	@param	[in]buf			缓冲区
 *	@param	[in]len			缓冲区大小
 *	@return 成功返回0，失败返回-1
 */
static int icmp_unpack(char *buf, int len);

/*
 *	@brief	发送ICMP回显请求报文
 *	@param	[in]argv		参数
 *	@return 
 */
static void *icmp_send(void *argv);

/*
 *	@brief	接收ICMP回显应答报文
 *	@param	[in]argv		参数
 *	@return 
 */
static void *icmp_recv(void *argv);

/*
 *	@brief	计算时间差time_sub
 *	@param	[in]begin		开始发送的时间
 *	@param  [in]end			接收到的时间
 *	@return 返回时间差
 */
static struct timeval icmp_tvsub(struct timeval begin, struct timeval end);

/*
 *	@brief	统计并打印全部的发送接收的ICMP包
 *	@return 
 */
static void icmp_statistics();

/*
 *	@brief	在ICMP包数组中查找指定的ICMP包
 *	@param	[in]seq			包的序列号
 *	@return 
 */
static ping_packet *icmp_findpacket(int seq);