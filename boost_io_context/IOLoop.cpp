#include "IOLoop.h"
#include "TimerQueue.h"
#include "CurrentThread.h"
#include <glog/logging.h>
#include <boost/asio/post.hpp>
#include <boost/asio/dispatch.hpp>

namespace IOEvent
{

//
// one loop per thread 顾名思义就是每个线程有且只有一个io_context,所以IOLoop的构造函数会检测当前线程是否已经创建了IOEvent
// 实现就是用到thread_local缓存一个IOLoop
//

thread_local IOLoop *t_inThisThreadLoop = nullptr;
IOLoop::IOLoop()
	:ioContext_(),
	timerQueue_(std::make_unique<TimerQueue>(this)),
	work_(make_work_guard(ioContext_)),
	threadId_(CurrentThread::tid()),
	quit_(false)	
{
	if (t_inThisThreadLoop)
	{
		LOG(FATAL) << "Another IOLoop :" << t_inThisThreadLoop << " exists in this thread :" << threadId_;
	}
	else
	{
		t_inThisThreadLoop = this;
	}
}

IOLoop::~IOLoop()
{
	LOG(WARNING) << "IOLoop :" << this << " of thread :" << threadId_
		<< " destructs in thread :" << CurrentThread::tid();
	if (!quit_)
	{
		quit();
	}
}

bool IOLoop::isInLoopThread() const
{
	return CurrentThread::tid() == threadId_;
}

void IOLoop::assertInLoopThread()
{
	if (!isInLoopThread())
	{
		abortNotInLoopThread();
	}
}

TimerId IOLoop::runAt(const Timestamp & when, TimerCallback cb)
{
	return timerQueue_->addTimer(std::move(cb), when, 0.0);
}

TimerId IOLoop::runAfter(double delay, TimerCallback cb)
{
	Timestamp time(addTime(Timestamp::now(), delay));
	return runAt(time, std::move(cb));
}

TimerId IOLoop::runEvery(double interval, TimerCallback cb)
{
	Timestamp time(addTime(Timestamp::now(), interval));
	return timerQueue_->addTimer(std::move(cb), time, interval);
}

void IOLoop::dispatch(FuncCallback cb)
{
	if (isInLoopThread())
	{
		cb();
	}
	else
	{
		post(cb);
	}
}

void IOLoop::post(FuncCallback cb)
{
	boost::asio::post(ioContext_, std::move(cb));
}


io_context *IOLoop::getContext()
{
	return &ioContext_;
}

void IOLoop::cancel(TimerId timerId)
{
	timerQueue_->cancel(std::move(timerId));
}

void IOLoop::loop()
{
	assertInLoopThread();
	ioContext_.run();
}

void IOLoop::quit()
{
	quit_ = true;	
	post(std::bind(&IOLoop::quitInThisThread, this));	
}

void IOLoop::abortNotInLoopThread()
{
	LOG(FATAL) << "IOLoop::abortNotInLoopThread - IOLoop " << this
		<< " was created in threadId_ = " << threadId_
		<< ", current thread id = " << CurrentThread::tid();
}

void IOLoop::quitInThisThread()
{
	assertInLoopThread();
	work_.reset();
	ioContext_.stop();
}

}
