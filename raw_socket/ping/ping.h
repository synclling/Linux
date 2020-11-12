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
#include <netinet/icmp.h>


#define SEND_BUFFER_SIZE 	72		// 发送缓冲区的大小
#define RECV_BUFFER_SIZE 	2048	// 接收缓冲区的大小


/* 保存已发送包的状态 */
typedef struct ping_packet {
	short 	seq;				// 序列号
	int 	flag;				// 1表示已发送但没有收到响应包，0表示收到响应包
	struct timeval tv_begin;	// 发送的时间
	struct timeval tv_end;		// 接收的时间
} ping_packet;


/*
 *	@brief	CRC16校验和计算
 *	@param	[in]data		数据
 *	@param	[in]len			数据长度
 *	@return 返回计算结果
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