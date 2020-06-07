#ifndef _IOEVENT_CURRENT_THREAD_H
#define _IOEVENT_CURRENT_THREAD_H

#include <thread>

namespace IOEvent
{

namespace CurrentThread
{
// internal
extern thread_local std::thread::id cached_tid;
extern thread_local const char* t_threadName;

void CacheTid();

inline std::thread::id tid()
{
	if (cached_tid == std::thread::id())
	{
		CacheTid();
	}
	return cached_tid;
}


}

}




#endif // !_IOEVENT_CURRENT_THREAD_H
