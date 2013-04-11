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

#endif // threadpool_h__
