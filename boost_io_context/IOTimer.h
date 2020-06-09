#ifndef _IOEVENT_IOTIMER_H
#define _IOEVENT_IOTIMER_H

#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/noncopyable.hpp>
#include <boost/system/error_code.hpp>
#include <functional>
#include <cassert>
#include <deque>
#include "Thread.h"


namespace IOEvent
{

template <typename Duration = boost::posix_time::milliseconds>
class IOTimer final : public boost::noncopyable
{
	using io_service = boost::asio::io_context;
	using deadline_timer = boost::asio::deadline_timer;
	using TimerCallback = std::function<void()>;
	using executor_work_guard = boost::asio::executor_work_guard<io_service::executor_type>;
public:
	IOTimer()
		:timer_(service_, Duration(0)),
		isSingleShot_(false),
		isActive_(false),
		thread_(std::bind(&IOTimer::threadFunc, this), "IOTimerThread")
	{
		work_ = make_work_guard(service_);
	}

	~IOTimer()
	{
		if (isActive_)
		{
			stop();
		}
	}
public:
	void start()
	{
		assert(!isActive_);
		assert(!thread_.started());
		isActive_ = true;
		thread_.start();
		run();
	}

	void stop()
	{
		assert(isActive_);
		isActive_ = false;
		work_.reset();
		service_.stop();
		thread_.join();
	}

	void addTimer(TimerCallback cb)
	{
		boost::asio::post(service_, std::bind(&IOTimer::addTimerInThisThread, this, std::move(cb)));
	}

	void setDuration(uint32_t  duration) { duration_ = duration; }

	void setSingleShot(bool isSingleShot) { isSingleShot_ = isSingleShot; }

private:
	void run()
	{
		timer_.expires_at(timer_.expires_at() + Duration(duration_));
		timer_.async_wait(std::bind(&IOTimer::timerHandle, this, std::placeholders::_1));
	}
	void threadFunc()
	{
		service_.run();
	}

	void timerHandle(const boost::system::error_code &error)
	{
		if (error == boost::system::errc::success)
		{
			for (auto &f : funcs_)
			{
				f();
			}
			if (!isSingleShot_)
			{
				run();
			}
		}
	}

	void addTimerInThisThread(const TimerCallback &cb)
	{
		funcs_.push_back(cb);
	}
private:
	io_service service_;
	executor_work_guard work_;
	deadline_timer timer_;
	std::deque<TimerCallback> funcs_;
	Thread thread_;
	uint32_t duration_;
	std::atomic<bool> isSingleShot_;
	std::atomic<bool> isActive_;
};

}















#endif // !_IOEVENT_IOTIMER_H
