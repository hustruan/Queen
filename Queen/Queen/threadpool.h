#ifndef threadpool_h__
#define threadpool_h__

#include <boost/threadpool.hpp>

using namespace boost::threadpool;

inline pool& GlobalThreadPool()
{
	static pool tp(boost::thread::hardware_concurrency());
	return tp;
}

template <typename Func>
inline void ScheduleAndJoin(pool& tp, Func func)
{
	for (size_t i = 0; i < tp.size() - 1; ++i)
	{
	tp.schedule(func);
	}
	
	// run on current thread 
	func();

	tp.wait();
}


#endif // threadpool_h__
