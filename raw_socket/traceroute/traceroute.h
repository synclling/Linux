#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>

#include <arpa/inet.h>
#include <netinet/ip_icmp.h>


#define TTL_LIMIT			30		// TTL上限
#define PACKETS_PER_HOP		3		// 每一跳发送的探测包数量
#define ICMP_HEADER_LEN		8		// ICMP报文头部长度
#define BUFFER_SIZE 		1024	// 缓冲区大小


unsigned char send_buf[BUFFER_SIZE];
unsigned char recv_buf[BUFFER_SIZE];

static pid_t pid;

uint16_t icmp_cksum(uint16_t *data, int len);

void icmp_pack(struct icmp *icmph, int seq);

void icmp_unpack(uint8_t *data, int len);

/*
 *	@brief	计算时间差
 *	@param	[in]start		开始时间
 *	@param  [in]end			结束时间
 *	@return 返回毫秒数
 */
double icmp_tvsub(struct timeval start, struct timeval end);
