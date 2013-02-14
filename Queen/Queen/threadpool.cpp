#include "threadpool.h"

boost::threadpool& GlobalThreadPool()
{
	static boost::threadpool tp(boost::thread::hardware_concurrency());
	return tp;
}