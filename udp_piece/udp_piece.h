#ifndef _UDP_PIECE_H_
#define _UDP_PIECE_H_

#include <stdint.h>


#define HEAD_POS_SYNC_WORD 		0 // 同步字
#define HEAD_POS_TOTAL_SIZE		2 // 所有分片的数据大小(不包括HEADA)
#define HEAD_POS_TOTAL_PIECES	4 // 分片的总数量
#define HEAD_POS_PIECE_INDEX	6 // 分片序号，从0开始
#define HEAD_POS_PIECE_SIZE		8 // 当前分片的数据大小


#define HEAD_SIZE 	12
#define PIECE_SIZE 	536 // MTU - UDP_HEAD - IP_HEAD - PIECE_HEAD (576 - 8 - 20 - 12)

typedef struct udp_piece_t
{
	int total_size;		// 总数据大小
	int total_pieces;	// 分片总数量
	int piece_size;		// 分片大小
	int left;			// 最后一片分片的大小

	int recv_pieces;	// 当前已经接收的分片数量

	int recv_size;		// 接收的数据长度
	uint8_t *recv_buf;	// 接收缓冲区
	uint8_t *send_ptr;	// 指向发送数据的buffer

	uint8_t *piece_buf[PIECE_SIZE];	// 单帧的buffer

	circular_buffer_t *circular_buffer;	// 环形缓冲区
};


/*
 * @brief 初始化资源
 * @param buf_size 设置环形缓冲区数据的最大长度
 * @return 成功则返回一个句柄，失败则返回NULL
 */
udp_piece_t *udp_piece_init(int buf_size);

/*
 * @brief 释放资源
 * @param udp_piece 句柄
 */
void udp_piece_uninit(udp_piece_t *udp_piece);

/*
 * @brief 重置，这里不会重新分配资源，只是讲部分参数重置到初始化状态
 * @param udp_piece 句柄
 */
void udp_piece_reset(udp_piece_t *udp_piece);

/*
 * @brief 根据长度进行切割，返回切割后的分片数量
 * @param udp_piece 句柄
 * @param buf       要分片数据的指针
 * @param size      要分片数据的长度
 * @return 返回分片的数量
 */
int udp_piece_cut(udp_piece_t *udp_piece, void *buf, int size);

/*
 * @brief 根据分片编号获取分片指针及分片数据大小
 * @param udp_piece 句柄
 * @param index     分片编号
 * @param got_piece_size 获取指定编号分片数据的长度
 * @return 返回指定分片编号的数据指针
 */
uint8_t *udp_piece_get(udp_piece_t *udp_piece, int index, int *got_piece_size);

/*
 * @brief 重组分片
 * @param udp_piece 句柄
 * @param buf   分片数据的指针
 * @param size  分片数据的长度
 * @return  返回-1则重组失败，返回0则正在重组中，返回1则重组成功
 */
int udp_piece_merge(udp_piece_t *udp_piece, void *buf, int size);

/*
 * @brief 重组分片，使用环形缓存
 * @param udp_piece 句柄
 * @param buf   分片数据的指针
 * @param size  分片数据的长度
 * @return  返回-1则重组失败，返回0则正在重组中，返回1则重组成功
 */
int udp_piece_merge_ex(udp_piece_t *udp_piece, void *buf, int size);

#endif
