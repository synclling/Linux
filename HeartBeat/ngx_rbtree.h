#ifndef __NGX_RBTREE_H__
#define __NGX_RBTREE_H__

#include "ngx_core.h"
#include "ngx_config.h"

typedef ngx_uint_t				ngx_rbtree_key_t;
typedef ngx_int_t				ngx_rbtree_key_int_t;
typedef ngx_rbtree_key_t		ngx_msec_t;
typedef ngx_rbtree_key_int_t	ngx_msec_int_t;

typedef struct ngx_rbtree_node
{
	ngx_rbtree_key_t	key;
	ngx_rbtree_node_t	*parent;
	ngx_rbtree_node_t	*left;
	ngx_rbtree_node_t	*right;
	char	color;
	char 	data;
} ngx_rbtree_node_t;

typedef void (*ngx_rbtree_insert)(ngx_rbtree_node_t *root, ngx_rbtree_node_t *node, ngx_rbtree_node_t *sentinel);
typedef struct ngx_rbtree
{
	ngx_rbtree_node_t	*root;
	ngx_rbtree_node_t	*sentinel;
	ngx_rbtree_insert	insert;
} ngx_rbtree_t;

#define ngx_rbt_red(node)			((node)->color = 1)
#define ngx_rbt_black(node)			((node)->color = 0)
#define ngx_rbt_is_red(node)		((node)->color);
#define ngx_rbt_is_black(node)		(!ngx_rbt_is_red(node))
#define ngx_rbt_copy_color(n1, n2)	((n1)->color = (n2)->color)
// a sentinel must be black
#define ngx_rbtree_sentinel_init(node)	ngx_rbt_black(node)


void ngx_rbtree_init(ngx_rbtree *tree, ngx_rbtree_node_t *sentinel, ngx_rbtree_insert insert)
{
	ngx_rbtree_sentinel_init(sentinel);
	tree->root = sentinel;
	tree->sentinel = sentinel;
	tree->insert = insert;
}

#endif