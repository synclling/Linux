#include "ngx_rbtree.h"


static inline void ngx_rbtree_left_rotate(ngx_rbtree_node_t **root, ngx_rbtree_node_t *sentinel, ngx_rbtree_node_t *node);
static inline void ngx_rbtree_right_rotate(ngx_rbtree_node_t **root, ngx_rbtree_node_t *sentinel, ngx_rbtree_node_t *node);



void ngx_rbtree_init(ngx_rbtree *tree, ngx_rbtree_node_t *sentinel, ngx_rbtree_insert insert)
{
	ngx_rbtree_sentinel_init(sentinel);
	tree->root = sentinel;
	tree->sentinel = sentinel;
	tree->insert = insert;
}

void ngx_rbtree_insert(ngx_rbtree_t *tree, ngx_rbtree_node_t *node)
{
	ngx_rbtree_node_t *temp = NULL;

	ngx_rbtree_node_t **root = &(tree->root);
	ngx_rbtree_node_t *sentinel = tree->sentinel;

	if(*root == sentinel) // 根节点为空
	{
		node->parent = NULL;
		node->left = sentinel;
		node->right = sentinel;
		ngx_rbt_black(node); // 设置根结点为黑色
		*root = node;

		return;
	}

	tree->insert(*root, node, sentinel); // 插入结点

	// re-balance tree
	while(node != *root && ngx_rbt_is_red(node->parent))
	{
		if(node->parent == node->parent->parent->left) // 当前结点的父结点为祖父结点的左孩子
		{
			temp = node->parent->parent->right; // 叔叔结点
			if(ngx_rbt_is_red(temp)) // 叔叔结点为红色
			{
				ngx_rbt_black(node->parent);
				ngx_rbt_black(temp);
				ngx_rbt_red(node->parent->parent);
				node = node->parent->parent;
			}
			else // 叔叔结点为黑色
			{
				if(node == node->parent->right)
				{
					node = node->parent;
					ngx_rbtree_left_rotate(root, sentinel, node);
				}

				ngx_rbt_black(node->parent);
				ngx_rbt_red(node->parent->parent);
				ngx_rbtree_right_rotate(root, sentinel, node->parent->parent);
			}
		}
		else // 当前结点的父节点为祖父结点的右孩子
		{
		}
	}
	
}



static inline void ngx_rbtree_left_rotate(ngx_rbtree_node_t **root, ngx_rbtree_node_t *sentinel, ngx_rbtree_node_t *node)
{
	ngx_rbtree_node_t *temp = node->right;

	node->right = temp->left;
	if(temp->left != sentinel)
	{
		temp->left->parent = node;
	}

	temp->parent = node->parent;
	
	if(node == *root)
	{
		*root = temp;
	}
	else if(node == node->parent->left)
	{
		node->parent->left = temp;
	}
	else
	{
		node->parent->right = temp;
	}

	temp->left = node;
	node->parent = temp;
}

static inline void ngx_rbtree_right_rotate(ngx_rbtree_node_t **root, ngx_rbtree_node_t *sentinel, ngx_rbtree_node_t *node)
{
	ngx_rbtree_node_t *temp = node->left;

	node->left = temp->right;
	if(temp->right != sentinel)
	{
		temp->right->parent = node;
	}

	temp->parent = node->parent;

	if(node == *root)
	{
		*root = temp;
	}
	else if(node == node->parent->left)
	{
		node->parent->left = temp;
	}
	else
	{
		node->parent->right = temp;
	}

	temp->right = node;
	node->parent = temp;
}
