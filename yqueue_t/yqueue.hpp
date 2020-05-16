#ifndef __ZMQ_YQUEUE_HPP_INCLUDED__
#define __ZMQ_YQUEUE_HPP_INCLUDED__

#include <stdlib.h>
#include <stddef.h>
#include <assert.h>

#include "atomic_ptr.hpp"

namespace zmq
{
template<typename T, int N> class yqueue_t
{
public:
	inline yqueue_t()
	{
		begin_pos = 0;
		begin_chunk = allocate_chunk();
		assert(begin_chunk);

		back_pos = 0;
		back_chunk = NULL;

		end_pos = 0;
		end_chunk = begin_chunk;
	}

	inline ~yqueue_t()
	{
		while(true)
		{
			if(begin_chunk == end_chunk)
			{
				free(begin_chunk);
				break;
			}

			chunk_t *o = begin_chunk;
			begin_chunk = begin_chunk->next;
			free(o);
		}

		chunk_t *sc = spare_chunk.xchg(NULL);
		free(sc);
	}

	inline T &front()
	{
		return begin_chunk.values[begin_pos];
	}

	inline T &back()
	{
		return back_chunk.values[back_pos];
	}

	inline void push()
	{
		back_chunk = end_chunk;
		back_pos = end_pos;

		if(++end_pos != N)
		{
			return;
		}

		chunk_t *sc = spare_chunk.xchg(NULL);
		if(sc != NULL)
		{
			end_chunk->next = sc;
			sc->prev = end_chunk;
		}
		else
		{
			end_chunk->next = allocate_chunk();
			assert(end_chunk->next);
			end_chunk->next->prev = end_chunk;
		}

		end_chunk = end_chunk->next;
		end_pos = 0;
	}

	inline void unpush()
	{
		if(back_pos > 0)
		{
			--back_pos;
		}
		else
		{
			back_pos = N - 1;
			back_chunk = back_chunk->prev;
		}

		if(end_pos > 0)
		{
			--end_pos;
		}
		else
		{
			end_pos = N - 1;
			end_chunk = end_chunk->prev;
			free(end_chunk->next);
			end_chunk->next = NULL;
		}
	}

	inline void pop()
	{
		if(++begin_pos == N)
		{
			chunk_t *o = begin_chunk;
			begin_chunk = begin_chunk->next;
			begin_chunk->prev = NULL;
			begin_pos = 0;

			chunk_t *cs = spare_chunk.xchg(o);
			free(cs);
		}
	}

private:
	struct chunk_t
	{
		T values[N];
		chunk_t *prev;
		chunk_t *next;
	};

	inline chunk_t *allocate_chunk()
	{
		return (chunk_t *)malloc(sizeof(chunk_t));
	}

	int begin_pos;
	chunk_t *begin_chunk;
	int back_pos;
	chunk_t *back_chunk;
	int end_pos;
	chunk_t *end_chunk;

	atomic_ptr_t<chunk_t> spare_chunk;

private:  // Disable copying of yqueue.
	yqueue_t(const yqueue_t &obj);
	const yqueue_t &operate=(const yqueue_t &obj);
};


};

#endif
