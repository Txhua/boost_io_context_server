#include "CurrentThread.h"

namespace IOEvent
{

namespace CurrentThread
{
thread_local std::thread::id cached_tid = std::thread::id();
thread_local const char* t_threadName = "unknow";
void CacheTid()
{
	cached_tid = std::this_thread::get_id();	
}
}
}