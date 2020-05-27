#ifndef __NGX_REACTOR_H__
#define __NGX_REACTOR_H__

typedef int (*ngx_event_handler)(ngx_event_t *ev);

struct ngx_event
{
	int sockfd;
	unsigned int event; // read, write, timeout and so on

	int status;
	int timer_set;
	int timeout;
	
	ngx_uint_t last_active;

	void *arg;
	ngx_event_handler handler;

	int size;
	char buffer[BUFFER_SIZE];
};

struct ngx_reactor
{
	
};

#endif