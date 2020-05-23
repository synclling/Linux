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

int udp_piece_merge(udp_piece_t *udp_piece, void *recv_buf, int recv_size)
{
	int get_all_pieces = 0;

	int index = 0;
	int temp_total_size = 0;
	int temp_total_pieces = 0;

	int temp_size = recv_size;
	uint8_t *temp_buf = (uint8_t *)recv_buf;

	for(int i = 0; i < recv_size; ++i)
	{
		if(temp_buf[0] == 0xAF && temp_buf[1] == 0xAE)
		{
			break;
		}

		++temp_buf;
		--temp_size;
	}

	while(temp_size > PIECE_HEAD_SIZE)
	{
		int data_size = (temp_buf[HEAD_POS_PIECE_DATA_SIZE] << 8) + temp_buf[HEAD_POS_PIECE_DATA_SIZE + 1];
		if(temp_size >= (PIECE_HEAD_SIZE + data_size))
		{
			index = (temp_buf[HEAD_POS_PIECE_INDEX] << 8) + temp_buf[HEAD_POS_PIECE_INDEX + 1];
			if(udp_piece->total_size == 0) // 第一个分片
			{
				udp_piece->total_size = (temp_buf[HEAD_POS_TOTAL_SIZE] << 8) + temp_buf[HEAD_POS_TOTAL_SIZE + 1];
				udp_piece->total_pieces = (temp_buf[HEAD_POS_TOTAL_PIECES] << 8) + temp_buf[HEAD_POS_TOTAL_PIECES + 1];

				udp_piece->recv_len = 0;
				udp_piece->recv_pieces = 0;

				if(udp_piece->recv_buf != NULL)
				{
					free(udp_piece->recv_buf);
					udp_piece->recv_buf = NULL;
				}

				udp_piece->recv_buf = (uint8_t *)malloc(udp_piece->total_size + 1);
				if(udp_piece->recv_buf == NULL)
				{
					printf("malloc recv_buf failed.\n");
					return -1;
				}
			}

			printf("recv_size: %d, piece_data_size: %d, index: %d, recv_pieces: %d, total_size: %d, total_pieces: %d\n",
				temp_size, data_size, index, udp_piece->recv_pieces, udp_piece->total_size, udp_piece->total_pieces);

			temp_total_size = (temp_buf[HEAD_POS_TOTAL_SIZE] << 8) + temp_buf[HEAD_POS_TOTAL_SIZE + 1];
			temp_total_pieces = (temp_buf[HEAD_POS_TOTAL_PIECES] << 8) + temp_buf[HEAD_POS_TOTAL_PIECES + 1];

			if(temp_total_size != udp_piece->total_size || temp_total_pieces != udp_piece->total_pieces)
			{
				udp_piece->total_size = temp_total_size;
				udp_piece->total_pieces = temp_total_pieces;

				udp_piece->recv_pieces = 1;
				udp_piece->recv_len = 0;
				if(udp_piece->recv_buf != NULL)
				{
					free(udp_piece->recv_buf);
					udp_piece->recv_buf = NULL;
				}
				udp_piece->recv_buf = (uint8_t *)malloc(udp_piece->total_size + 1);
				if(udp_piece->recv_buf != NULL)
				{
					printf("malloc recv_buf failed.\n");
					return -1;
				}
			}

			temp_buf += PIECE_HEAD_SIZE;
			temp_size -= PIECE_HEAD_SIZE;

			memcpy(&udp_piece->recv_buf[index * PIECE_DATA_SIZE], temp_buf, data_size);

			temp_buf += data_size;
			temp_size -= data_size;

			udp_piece->recv_len += data_size;
			++udp_piece->recv_pieces;
			if(udp_piece->recv_pieces == udp_piece->total_pieces)
			{
				udp_piece->total_pieces = 0;
				udp_piece->recv_pieces = 0;

				if(udp_piece->recv_len == udp_piece->total_size)
				{
					get_all_pieces = 1;
				}
				else
				{
					printf("recv_len != total_size! recv_len: %d, total_size: %d\n",
						udp_piece->recv_len, udp_piece->total_size);
					get_all_pieces = -1;
				}
			}
		}
		else
		{
			temp_size = 0;
		}
	}

	return get_all_pieces;
}

