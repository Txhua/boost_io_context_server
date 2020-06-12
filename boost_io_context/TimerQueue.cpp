#include "TimerQueue.h"
#include "Timer.h"
#include "IOLoop.h"
#include <boost/asio/post.hpp>
#include <cassert>

namespace IOEvent
{
TimerQueue::TimerQueue(IOLoop *loop)
	:baseLoop_(loop),
	steadyTimer_(*baseLoop_->getContext()),
	timers_(),
	callingExpiredTimers_(false)
{
}

TimerQueue::~TimerQueue()
{
	steadyTimer_.cancel();
	for (auto &entry : timers_)
	{
		delete entry.second;
	}
}

TimerId TimerQueue::addTimer(TimerCallback cb, const Timestamp & when, double interval)
{
	auto *timer = new Timer(std::move(cb), when, interval);
	boost::asio::post(*baseLoop_->getContext(), std::bind(&TimerQueue::addTimerInThisThread, this, timer));
	return TimerId(timer, timer->sequence());
}
void TimerQueue::cancel(TimerId timerId)
{
	boost::asio::post(*baseLoop_->getContext(), std::bind(&TimerQueue::cancelInThisThread, this, timerId));
}

void TimerQueue::addTimerInThisThread(Timer * timer)
{
	bool earliestChanged = insert(timer);
	if (earliestChanged)
	{
		resetTimer(timer->expiration());
	}
}

void TimerQueue::cancelInThisThread(TimerId timerId)
{
	assert(timers_.size() == activeTimers_.size());
	ActiveTimer timer(timerId.timer_, timerId.sequence_);
	auto iter = activeTimers_.find(timer);
	if (iter != activeTimers_.end())
	{
		size_t n = timers_.erase(Entry(iter->first->expiration(), iter->first));
		assert(n == 1); (void)n;
		delete iter->first;
		activeTimers_.erase(iter);
	}
	else if (callingExpiredTimers_)
	{
		cancelingTimers_.emplace(timer);
	}
	assert(timers_.size() == activeTimers_.size());
}

bool TimerQueue::insert(Timer * timer)
{
	assert(timers_.size() == activeTimers_.size());
	bool earliestChanged = false;
	const Timestamp &when = timer->expiration();
	auto iter = timers_.begin();
	if (iter == timers_.end() || when < iter->first)
	{
		earliestChanged = true;
	}
	{
		auto result = timers_.emplace(Entry(when, timer));
		assert(result.second); (void)result;
	}
	{		
		auto result = activeTimers_.emplace(ActiveTimer(timer, timer->sequence()));
		assert(result.second); (void)result;
	}
	assert(timers_.size() == activeTimers_.size());
	return earliestChanged;
}

void TimerQueue::resetTimer(const Timestamp & when)
{
	int64_t microSeconds = when.microSecondsSinceEpoch() - Timestamp::now().microSecondsSinceEpoch();
	if (microSeconds < 100)
	{
		microSeconds = 100;
	}
	time_t tv_sec = static_cast<time_t>(microSeconds / Timestamp::kMicroSecondsPerSecond);
	update(tv_sec);
}

void TimerQueue::update(time_t sec)
{
	steadyTimer_.expires_from_now(std::chrono::seconds(sec));
	steadyTimer_.async_wait([&](const boost::system::error_code &error) 
	{
		if (!error)
		{
			// 获取过期的时间
			auto now = Timestamp::now();
			auto expired = getExpired(now);
			// 执行过期任务
			cancelingTimers_.clear();
			callingExpiredTimers_ = true;
			for (auto &timer : expired)
			{
				timer.second->run();
			}
			callingExpiredTimers_ = false;
			// 重新设置定时器
			reset(expired, now);
		}
	});
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(const Timestamp & now)
{
	assert(timers_.size() == activeTimers_.size());
	std::vector<Entry> expired;
	Entry entry(now, reinterpret_cast<Timer *>(UINTPTR_MAX));
	auto end = timers_.lower_bound(entry);
	assert(end == timers_.end() || now < end->first);
	std::copy(timers_.begin(), end, std::back_inserter(expired));
	// 删除过期的时间
	timers_.erase(timers_.begin(), end);
	// 删除过期的活动时间
	for (auto &exp : expired)
	{
		ActiveTimer timer(exp.second, exp.second->sequence());
		size_t n = activeTimers_.erase(timer);
		assert(n == 1); (void)n;
	}
	assert(timers_.size() == activeTimers_.size());
	return expired;
}

void TimerQueue::reset(const std::vector<Entry>& expired, const Timestamp &now)
{
	Timestamp nextExpire;
	for (auto &entry : expired)
	{
		ActiveTimer timer(entry.second, entry.second->sequence());
		if (entry.second->repeat() && cancelingTimers_.find(timer) == cancelingTimers_.end())
		{
			entry.second->restart(now);
			insert(entry.second);
		}
		else
		{
			delete entry.second;
		}
	}

	if (!timers_.empty())
	{
		nextExpire = timers_.begin()->second->expiration();
	}

	if (nextExpire.vaild())
	{
		resetTimer(nextExpire);
	}
}

}
