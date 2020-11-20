#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>

#include <arpa/inet.h>
#include <netinet/ip_icmp.h>


#define TIMEOUT				1000	// miliseconds(1000 ms = 1 s)
#define TTL_LIMIT			30
#define REQUEST_PER_TTL		3
#define BUFFER_SIZE 		1024
#define ICMP_HEADER_LEN		8

char sendbuf[BUFFER_SIZE];
char recvbuf[BUFFER_SIZE];

static pid_t pid;

uint16_t icmp_cksum(uint16_t *data, int len);

void icmp_pack(struct icmp *icmph, int seq);

double time_difference(struct timeval begin, struct timeval end);
