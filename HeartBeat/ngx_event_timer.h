/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_EVENT_TIMER_H_INCLUDED_
#define _NGX_EVENT_TIMER_H_INCLUDED_


#include "ngx_config.h"
#include "ngx_core.h"


#define NGX_TIMER_INFINITE  (ngx_msec_t) -1

#define NGX_TIMER_LAZY_DELAY  300


ngx_int_t ngx_event_timer_init(void);
ngx_msec_t ngx_event_find_timer(void);
void ngx_event_expire_timers(void);
ngx_int_t ngx_event_no_timers_left(void);


extern ngx_rbtree_t  ngx_event_timer_rbtree;
extern volatile ngx_msec_t  ngx_current_msec;


static ngx_inline void
ngx_event_del_timer(ngx_event_t *ev)
{
    printf( "event timer del: %d: %ld\n",
            ev->fd, ev->timer.key);

    ngx_rbtree_delete(&ngx_event_timer_rbtree, &ev->timer);

    ev->timer.left = NULL;
    ev->timer.right = NULL;
    ev->timer.parent = NULL;
    ev->timer_set = 0;
}


static ngx_inline void
ngx_event_add_timer(ngx_event_t *ev, ngx_msec_t timer)
{
    ngx_msec_t      key;
    ngx_msec_int_t  diff;

    key = ngx_current_msec + timer;

    if (ev->timer_set)
	{

        /*
         * Use a previous timer value if difference between it and a new
         * value is less than NGX_TIMER_LAZY_DELAY milliseconds: this allows
         * to minimize the rbtree operations for fast connections.
         */

        diff = (ngx_msec_int_t) (key - ev->timer.key);

        if (ngx_abs(diff) < NGX_TIMER_LAZY_DELAY)
		{
            printf( "event timer: %d, old: %ld, new: %ld\n",
                    ev->fd, ev->timer.key, key);
            return;
        }

        ngx_event_del_timer(ev);
    }

    ev->timer.key = key;

    printf( "event timer add: %d: %ld:%ld\n",
            ev->fd, timer, ev->timer.key);

    ngx_rbtree_insert(&ngx_event_timer_rbtree, &ev->timer);

    ev->timer_set = 1;
}


#endif /* _NGX_EVENT_TIMER_H_INCLUDED_ */