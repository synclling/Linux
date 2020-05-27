#ifndef __NGX_CORE_H__
#define __NGX_CORE_H__

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>


#define NGX_EVENT_READING	0x01	/**< error encountered while reading */
#define NGX_EVENT_WRITING	0x02	/**< error encountered while writing */
#define NGX_EVENT_EOF		0x10	/**< eof file reached */
#define NGX_EVENT_ERROR		0x20	/**< unrecoverable error encountered */
#define NGX_EVENT_TIMEOUT	0x40	/**< user-specified timeout reached */
#define NGX_EVENT_CONNECTED	0x80	/**< connect operation finished. */

#define MAX_EPOLL_EVENTS	1024
#define BUFFER_SIZE			4096

typedef struct ngx_event ngx_event_t;
typedef struct ngx_reactor ngx_reactor_t;

#endif