#include "udp_piece.h"

udp_piece_t *udp_piece_init(int buf_size)
{
	udp_piece_t *udp_piece = (udp_piece_t *)malloc(sizeof(struct udp_piece_t));
	if(udp_piece == NULL)
	{
		return NULL;
	}

	memset(udp_piece_t, 0, sizeof(struct udp_piece_t));

	udp_piece->circular_buffer = circular_buffer_init(buf_size);
	if(udp_piece->circular_buffer == NULL)
	{
		free(udp_piece);
		return NULL;
	}

	return udp_piece;
}

void udp_piece_uninit(udp_piece_t *udp_piece)
{
	if(udp_piece == NULL)
	{
		return;
	}

	udp_piece->total_size = 0;
	udp_piece->total_pieces = 0;
	udp_piece->piece_size = 0;
	udp_piece->left = 0;
	udp_piece->recv_pieces = 0;
	udp_piece->recv_size = 0;

	if(udp_piece->recv_buf != NULL)
	{
		free(udp_piece->recv_buf);
		udp_piece->recv_buf = NULL;
	}
	
	udp_piece->send_ptr = NULL;

	circular_buffer_uninit(udp_piece->circular_buffer);
	udp_piece->circular_buffer = NULL;
}

int udp_piece_cut(udp_piece_t *udp_piece, void *buf, int size)
{
	if(udp_piece == NULL || size < 0)
	{
		return 0;
	}

	udp_piece->send_ptr = buf;
	udp_piece->total_size = size;
	udp_piece->left = size % PIECE_SIZE;
	udp_piece->total_pieces = udp_piece->left > 0? (size / PIECE_SIZE + 1) : (size / PIECE_SIZE);

	return udp_piece->total_pieces;
}

uint8_t *udp_piece_get(udp_piece_t *udp_piece, int index, int *got_piece_size)
{
	if(udp_piece == NULL || index < 0 || got_piece_size == NULL)
	{
		return NULL;
	}
	
	//#define HEAD_POS_SYNC_WORD 		0 // 同步字
	//#define HEAD_POS_TOTAL_SIZE		2 // 所有分片的数据大小(不包括HEADA)
	//#define HEAD_POS_TOTAL_PIECES	4 // 分片的总数量
	//#define HEAD_POS_PIECE_INDEX	6 // 分片序号，从0开始
	//#define HEAD_POS_PIECE_SIZE		8 // 当前分片的数据大小

	int piece_size = 0;
	if(index == udp_piece->total_pieces - 1 && udp_piece->left > 0)
	{
		piece_size = udp_piece->left;
	}
	else
	{
		piece_size = PIECE_SIZE;
	}

	memset(udp_piece->piece_buf, 0, sizeof(udp_piece->piece_buf));

	udp_piece->piece_buf[HEAD_POS_SYNC_WORD] = 0xAF;
	udp_piece->piece_buf[HEAD_POS_SYNC_WORD + 1] = 0xAE;

	udp_piece->piece_buf[HEAD_POS_TOTAL_SIZE] = (udp_piece->total_size >> 8);
	udp_piece->piece_buf[HEAD_POS_TOTAL_SIZE + 1] = (udp_piece->total_size & 0xff);

	udp_piece->piece_buf[HEAD_POS_TOTAL_PIECES] = (udp_piece->total_pieces >> 8);
	udp_piece->piece_buf[HEAD_POS_TOTAL_PIECES + 1] = (udp_piece->total_pieces & 0xff);

	udp_piece->piece_buf[HEAD_POS_PIECE_INDEX] = (index >> 8);
	udp_piece->piece_buf[HEAD_POS_PIECE_INDEX + 1] = (index & 0xff);

	udp_piece->piece_buf[HEAD_POS_PIECE_SIZE] = (piece_size >> 8);
	udp_piece->piece_buf[HEAD_POS_PIECE_SIZE + 1] = (piece_size & 0xff);

	// 拷贝用户数据到分片数据区
	memcpy(&udp_piece->piece_buf[HEAD_SIZE], &udp_piece->send_ptr[PIECE_SIZE * index], piece_size);
	*got_piece_size = piece_size + HEAD_SIZE;

	return udp_piece->piece_buf;
	
}

int udp_piece_merge(udp_piece_t *udp_piece, void *buf, int size)
{
	uint8_t *piece_buf = NULL;
	
}