#include "CurrentThread.h"

namespace IOEvent
{

namespace CurrentThread
{
thread_local std::thread::id t_thread_id = std::thread::id();
void cacheId()
{
	t_thread_id = std::this_thread::get_id();
}
std::thread::id tid()
{
	if (t_thread_id == std::thread::id())
	{
		cacheId();
	}
	return t_thread_id;
}
}


}