#ifndef __NGX_CONFIG_H__
#define __NGX_CONFIG_H__


#ifndef offsetof
#define offsetof(type, member) ((size_t) & ((type *)0)->member)
#endif

#define __WORDSIZE 64
#if __WORDSIZE == 64

#ifndef __intptr_t_defined
#define __intptr_t_defined
typedef long int 			intptr_t;
typedef unsigned long int 	uintptr_t;
#endif

#else

#ifndef __intptr_t_defined
#define __intptr_t_defined
typedef int				intptr_t;
typedef unsigned int	uintptr_t;
#endif

#endif

typedef unsigned char		ngx_char_t;
typedef unsigned short		ngx_short_t;
typedef unsigned long		ngx_long_t;
typedef intptr_t			ngx_int_t;
typedef uintptr_t			ngx_uint_t;
typedef intptr_t			ngx_flag_t;

#endif