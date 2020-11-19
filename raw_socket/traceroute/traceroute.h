#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <errno.h>
#include <sys/time.h>

#include <arpa/inet.h>
#include <netinet/ip_icmp.h>


#define TTL_LIMIT		30
#define REQUEST_PER_TTL	3
#define BUFFER_SIZE 	1024

char send_buf[BUFFER_SIZE];
char recv_buf[BUFFER_SIZE];


uint16_t in_cksum(uint16_t *addr, int len, int csum);


