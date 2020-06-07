#include "Thread.h"
#include <iostream>
#include <memory>
#include <assert.h>
#include "CurrentThread.h"
#include <glog/logging.h>

namespace IOEvent
{

class ThreadIdInitializer
{
public:
	ThreadIdInitializer()
	{
		CurrentThread::t_threadName = "Main";
		CurrentThread::tid();
	}
};

ThreadIdInitializer init;


struct ThreadData
{
	using ThreadFunc = Thread::ThreadFunc;
	explicit ThreadData(ThreadFunc func, const std::string &name, CountDownLatch *latch, std::thread::id *tid)
		:func_(std::move(func)),
		name_(name),
		latch_(latch),
		tid_(tid)
	{}

	~ThreadData()
	{
		LOG(WARNING) << "thread exit: " << CurrentThread::t_threadName;
	}

	void runInThread()
	{
		*tid_ = CurrentThread::tid();
		tid_ = nullptr;
		latch_->countDown();
		latch_ = nullptr;
		CurrentThread::t_threadName = name_.empty() ? "IOEventThread" : name_.data();
		try
		{
			func_();
			CurrentThread::t_threadName = "Finished";
		}
		catch (const std::exception &e)
		{
			CurrentThread::t_threadName = "crashed";
			LOG(FATAL) << "exception caught in Thread :" << name_.data() << " reason" << e.what();
		}
	}
private:
	ThreadFunc func_;
	CountDownLatch *latch_;
	std::thread::id *tid_;
	std::string name_;
};

void threadStart(const std::shared_ptr<ThreadData> &data)
{
	data->runInThread();
}

Thread::Thread(ThreadFunc func, const std::string & name)
	:func_(std::move(func)),
	name_(name),
	started_(false),
	join_(false),
	latch_(1),
	tid_(std::thread::id())
{
}

Thread::~Thread()
{
	if (started_ && !join_)
	{
		thread_.detach();
	}
}

void Thread::start()
{
	assert(!started_);
	started_ = true;
	auto threadData = std::make_shared<ThreadData>(func_, name_, &latch_, &tid_);
	thread_ = std::move(std::thread(std::bind(&threadStart, std::move(threadData))));
	latch_.wait();
	assert(tid_ != std::thread::id());
}

void Thread::join()
{
	assert(started_);
	assert(!join_);
	join_ = true;
	thread_.join();
}

}
