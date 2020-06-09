#include "Timer.h"

namespace IOEvent
{
std::atomic<int64_t> Timer::s_numCreated_ = 0;

Timer::Timer(TimerCallback cb, const Timestamp & expiration, double interval)
	:callback_(std::move(cb)),
	expiration_(expiration),
	interval_(interval),
	repeat_(interval_ > 0.0 ? true : false),
	sequence_(++s_numCreated_)
{
}

void Timer::restart(const Timestamp &now)
{
	if (repeat_)
	{
		expiration_ = addTime(now, interval_);
	}
	else
	{
		expiration_ = Timestamp::invalid();
	}
}

}