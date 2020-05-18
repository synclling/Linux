#include "udp_piece.h"

udp_piece_t *udp_piece_init(int buf_size)
{
	udp_piece_t *udp_piece = (udp_piece_t *)malloc(sizeof(udp_piece_t));
	if(udp_piece == NULL)
	{
		return NULL;
	}

	memset(udp_piece, 0, sizeof(udp_piece_t));

	udp_piece->circular_buf = circular_buffer_init(buf_size);
	if(udp_piece->circular_buf == NULL)
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

	udp_piece->piece_size = 0;
	udp_piece->total_size = 0;
	udp_piece->total_pieces = 0;
	udp_piece->last_piece = 0;

	udp_piece->recv_pieces = 0;

	udp_piece->recv_len = 0;
	udp_piece->recv_buf = NULL;
	udp_piece->send_ptr = NULL;

	circular_buffer_uninit(udp_piece->circular_buf);
	udp_piece->circular_buf = NULL;

}

int udp_piece_cut(udp_piece_t *udp_piece, void *data_buf, int data_size)
{
	if(udp_piece == NULL || data_buf == NULL || data_size < 0)
	{
		return 0;
	}

	udp_piece->total_size = data_size;
	udp_piece->last_piece = data_size % PIECE_DATA_SIZE;
	udp_piece->total_pieces = data_size / PIECE_DATA_SIZE;
	if(udp_piece->last_piece > 0)
	{
		++udp_piece->total_pieces;
	}

	udp_piece->send_ptr = data_buf;

	return udp_piece->total_pieces;
}

uint8_t *udp_piece_get(udp_piece_t *udp_piece, int index, int *got_piece_size)
{
	if(udp_piece == NULL || index < 0 || got_piece_size == NULL)
	{
		return NULL;
	}

	int piece_data_size = 0;
	if(index == (udp_piece->total_pieces - 1) && udp_piece->last_piece > 0)
	{
		piece_data_size = udp_piece->last_piece;
	}
	else
	{
		piece_data_size = PIECE_DATA_SIZE;
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

	udp_piece->piece_buf[HEAD_POS_PIECE_DATA_SIZE] = (piece_data_size >> 8);
	udp_piece->piece_buf[HEAD_POS_PIECE_DATA_SIZE + 1] = (piece_data_size & 0xff);

	memcpy(&udp_piece->piece_buf[PIECE_HEAD_SIZE], &udp_piece->send_ptr[index * PIECE_DATA_SIZE], piece_data_size);

	*got_piece_size = piece_data_size + PIECE_HEAD_SIZE;

	return udp_piece;
}

int udp_piece_merge(udp_piece_t *udp_piece, void *data_buf, int data_size)
{
	int get_all_pieces = 0;
	
	int index = 0;
	int temp_total_size = 0;
	int temp_total_pieces = 0;

	int temp_data_size = data_size;
	uint8_t *temp_data_buf = data_buf;
	for(int i = 0; i < data_size; ++i)
	{
		if(temp_data_buf[0] == 0xAF && temp_data_buf[1] == 0xAE)
		{
			break;
		}
		++temp_data_buf;
		--temp_data_size;
	}

	while(temp_data_size > PIECE_HEAD_SIZE)
	{
		int data_len = (temp_data_buf[HEAD_POS_PIECE_DATA_SIZE] << 8) + temp_data_buf[HEAD_POS_PIECE_DATA_SIZE + 1];
		
	}
	
}

