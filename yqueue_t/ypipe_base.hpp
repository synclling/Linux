#ifndef __ZMQ_YPIPE_BASE_HPP_INCLUDED__
#define __ZMQ_YPIPE_BASE_HPP_INCLUDED__

namespace zmq
{
	template<typename T> class ypipe_base_t
	{
	public:
		virtual ~ypipe_base_t(){}
		virtual void write (const T &value, bool incomplete) = 0;
		virtual bool unwrite (T *value) = 0;
		virtual bool flush () = 0;
		virtual bool check_read () = 0;
		virtual bool read (T *value) = 0;
		virtual bool probe (bool (*fn) (const T &)) = 0;
	};
}
#endif
