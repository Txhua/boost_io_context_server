#ifndef _IOEVENT_TIMER_QUEUE_H
#define _IOEVENT_TIMER_QUEUE_H

#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <set>
#include <vector>
#include <atomic>
#include "Timestamp.h"
#include "Callbacks.h"
#include "TimerId.h"

namespace IOEvent
{
using namespace boost::asio;
class Timer;
class TimerQueue
{
public:
	explicit TimerQueue(io_context &ios);
	~TimerQueue();
	TimerId addTimer(TimerCallback cb, const Timestamp &when, double interval);
	void cancel(TimerId timerId);
private:
	using Entry = std::pair<Timestamp, Timer*>;
	using TimerList = std::set<Entry>;
	using ActiveTimer = std::pair<Timer*, int64_t>;
	using ActiveTimerSet = std::set<ActiveTimer>;

	void addTimerInThisThread(Timer *timer);
	void cancelInThisThread(TimerId timerId);
	bool insert(Timer *timer);
	void resetTimer(const Timestamp &now);
	void update(time_t sec);
	std::vector<Entry> getExpired(const Timestamp &now);
	void reset(const std::vector<Entry> &expired, const Timestamp &now);
private:
	io_context &baseIoContext_;
	steady_timer steadyTimer_;
	TimerList timers_;
	ActiveTimerSet activeTimers_;
	std::atomic<bool> callingExpiredTimers_;
	ActiveTimerSet cancelingTimers_;
};

}

#endif // !_IOEVENT_TIMER_QUEUE_H

