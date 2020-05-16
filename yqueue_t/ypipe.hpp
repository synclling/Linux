#ifndef __ZMQ_YPIPE_HPP_INCLUDED__
#define __ZMQ_YPIPE_HPP_INCLUDED__

#include "atomic_ptr.hpp"
#include "yqueue.h"
#include "ypipe_base.hpp"

#include <assert.h>

namespace
{
template<typename T, int N> class ypipe_t : public ypipe_base_t<T>
{
public:
	inline ypipe_t()
	{
		queue.push();
		r = w = f = &queue.back();
		c.set(&queue.back());
	}

	inline virtual ~ypipe_t(){}

	inline void write(const T &value, bool incomplete)
	{
		queue.back() = value;
		queue.push();

		if(!incomplete)
		{
			f = &queue.back();
		}
	}

	inline bool unwrite(T *value)
	{
		if(f == &queue.back())
		{
			return false;
		}

		queue.unpush();
		*value = queue.back();

		return true;
	}

	inline bool flush()
	{
		if(w == f)
		{
			return true;
		}

		if(c.cas(w, f) != w)
		{
			c.set(f);
			w = f;
			return false;
		}

		w = f;
		return true;
	}

	inline bool check_read()
	{
		if(&queue.front() != r && r)
		{
			return true;
		}

		r = c.cas(&queue.front(), NULL);

		if(&queue.front() == r || !r)
		{
			return false;
		}

		return true;
	}

	inline bool read(T *value)
	{
		if(!check_read())
		{
			return false;
		}

		*value = queue.front();
		queue.pop();

		return true;
	}

	inline bool probe(bool (*fn)(const T &))
	{
		bool rc = check_read();
		assert(rc);

		return (*fn)(queue.front());
	}
	
protected:
	yqueue_t<T, N> queue;

	T *w;
	T *r;
	T *f;

	atomic_ptr_t<T> c;

private:
	ypipe_t(const ypipe_t &obj);
	const ypipe_t &operator=(const ypipe_t &obj);
};
}

#endif
