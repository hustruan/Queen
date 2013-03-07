#ifndef threadpool_h__
#define threadpool_h__

#include <boost/threadpool.hpp>

using namespace boost::threadpool;

inline pool& GlobalThreadPool()
{
	static pool tp(boost::thread::hardware_concurrency()-1);
	return tp;
}

inline uint32_t GetNumWorkThreads()
{
	return boost::thread::hardware_concurrency();
}

template <typename Func>
inline void ScheduleAndJoin(pool& tp, Func func)
{
	for (size_t i = 0; i < tp.size(); ++i)
	{
		tp.schedule(func);
	}
	
	// run on current thread 
	func();

	tp.wait();
}

#endif // threadpool_h__
