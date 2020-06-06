#ifndef _IOEVENT_CURRENT_THREAD_H
#define _IOEVENT_CURRENT_THREAD_H

#include <thread>

namespace IOEvent
{

namespace CurrentThread
{
extern thread_local std::thread::id t_thread_id;

void cacheId();
std::thread::id tid();
}

}




#endif // !_IOEVENT_CURRENT_THREAD_H
